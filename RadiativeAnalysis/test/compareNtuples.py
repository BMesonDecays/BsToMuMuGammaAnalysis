#!/usr/bin/env python3
"""Regression check: compare the branches of two ntuples (e.g. the reference and a new version).

Usage:
    python3 compareNtuples.py reference.root new.root [--tree BMMGTree] [--tree-new BMMGTree]
                              [--map old:new ...] [--rtol 1e-6] [--show 20] [--only-diff] [--entries N] [--ignore b1 b2 ...]

For every branch present in both files (after applying the --map renames) it reports whether
the values are identical event by event. If they are not, it gives the fraction of differing
entries and the mean/std in both files. Branches present in only one file are listed.
Events are compared in file order, so both ntuples must be made from the same input events.
Exit code 0 if all common branches agree, 1 otherwise.
"""
import argparse
import sys

import awkward as ak
import numpy as np
import uproot


def flat(arr):
    """Flatten any jagged/fixed-size array to 1D float64, NaN-safe."""
    a = ak.to_numpy(ak.flatten(arr, axis=None)) if arr.ndim > 1 else ak.to_numpy(arr)
    return a.astype(np.float64, copy=False)


def compare_branch(a, b, rtol):
    """Return (identical, n_diff_events, n_events, summary) for two awkward arrays."""
    if len(a) != len(b):
        return False, None, None, "different number of entries %d vs %d" % (len(a), len(b))
    if a.ndim > 1 or b.ndim > 1:
        na = ak.num(a, axis=1) if a.ndim > 1 else None
        nb = ak.num(b, axis=1) if b.ndim > 1 else None
        if na is None or nb is None or not ak.all(na == nb):
            ndiff = len(a) if na is None or nb is None else int(ak.sum(na != nb))
            return False, ndiff, len(a), "different multiplicities in %d events" % ndiff
        # same structure: compare element-wise, then reduce per event
        fa = ak.values_astype(a, np.float64)
        fb = ak.values_astype(b, np.float64)
        close = np.isclose(ak.to_numpy(ak.flatten(fa, axis=None)), ak.to_numpy(ak.flatten(fb, axis=None)),
                           rtol=rtol, atol=0.0, equal_nan=True)
        per_event = ak.unflatten(close, ak.flatten(ak.num(fa, axis=-1), axis=None)) if fa.ndim > 1 else close
        bad = ~ak.all(per_event, axis=-1)
        while bad.ndim > 1:
            bad = ak.any(bad, axis=-1)
        ndiff = int(ak.sum(bad))
        return ndiff == 0, ndiff, len(a), ""
    fa, fb = flat(a), flat(b)
    close = np.isclose(fa, fb, rtol=rtol, atol=0.0, equal_nan=True)
    ndiff = int(np.sum(~close))
    return ndiff == 0, ndiff, len(fa), ""


def stats(arr):
    v = flat(arr)
    v = v[np.isfinite(v)]
    if v.size == 0:
        return "n=0"
    return "mean=%.6g std=%.6g" % (v.mean(), v.std())


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("ref")
    ap.add_argument("new")
    ap.add_argument("--tree", default="BMMGTree", help="tree name in the reference file")
    ap.add_argument("--tree-new", default=None, help="tree name in the new file (default: same as --tree)")
    ap.add_argument("--map", nargs="*", default=[], help="branch renames old:new")
    ap.add_argument("--rtol", type=float, default=1e-6, help="relative tolerance for float comparison")
    ap.add_argument("--show", type=int, default=-1, help="max number of differing branches to print (-1 = all)")
    ap.add_argument("--only-diff", action="store_true", help="do not list identical branches")
    ap.add_argument("--ignore", nargs="*", default=[],
                    help="branches (reference names) to skip, e.g. known non-deterministic ones")
    ap.add_argument("--entries", type=int, default=-1,
                    help="compare only the first N entries (e.g. to skip the extra endJob row of the old ntuple)")
    args = ap.parse_args()

    tref = uproot.open(args.ref)[args.tree]
    tnew = uproot.open(args.new)[args.tree_new or args.tree]
    rename = dict(m.split(":", 1) for m in args.map)

    ref_names = set(tref.keys())
    new_names = set(tnew.keys())
    ignored = set(args.ignore)
    pairs = [(n, rename.get(n, n)) for n in sorted(ref_names)
             if rename.get(n, n) in new_names and n not in ignored]
    only_ref = sorted(n for n in ref_names if rename.get(n, n) not in new_names)
    mapped_new = {rename.get(n, n) for n in ref_names}
    only_new = sorted(n for n in new_names if n not in mapped_new)

    print("reference: %s (%d entries, %d branches)" % (args.ref, tref.num_entries, len(ref_names)))
    print("new      : %s (%d entries, %d branches)" % (args.new, tnew.num_entries, len(new_names)))
    if tref.num_entries != tnew.num_entries:
        print("WARNING: different number of entries; event-by-event comparison uses the first %d"
              % min(tref.num_entries, tnew.num_entries))
    n = min(tref.num_entries, tnew.num_entries)
    if args.entries >= 0:
        n = min(n, args.entries)
    print("comparing the first %d entries" % n)

    identical, different, failed = [], [], []
    for old, new in pairs:
        try:
            a = tref[old].array(entry_stop=n, library="ak")
            b = tnew[new].array(entry_stop=n, library="ak")
            same, ndiff, ntot, msg = compare_branch(a, b, args.rtol)
        except Exception as e:  # unsupported branch type etc.
            failed.append((old, str(e).splitlines()[0]))
            continue
        if same:
            identical.append(old)
        else:
            label = old if old == new else "%s -> %s" % (old, new)
            detail = msg or "%d/%d values differ" % (ndiff, ntot)
            different.append("%-55s %s | ref %s | new %s" % (label, detail, stats(a), stats(b)))

    print("\n%d common branches compared (%d ignored): %d identical, %d different, %d not comparable"
          % (len(pairs), len(ignored & ref_names), len(identical), len(different), len(failed)))
    if different:
        print("\n--- different ---")
        for line in different[: None if args.show < 0 else args.show]:
            print(line)
    if failed:
        print("\n--- not comparable ---")
        for name, err in failed:
            print("%-55s %s" % (name, err))
    if only_ref:
        print("\n--- only in reference (%d) ---\n%s" % (len(only_ref), " ".join(only_ref)))
    if only_new:
        print("\n--- only in new (%d) ---\n%s" % (len(only_new), " ".join(only_new)))
    if identical and not args.only_diff:
        print("\n--- identical (%d) ---\n%s" % (len(identical), " ".join(identical)))
    return 0 if not different and not failed else 1


if __name__ == "__main__":
    sys.exit(main())
