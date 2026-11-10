"""End-to-end test of convert.py on tiny synthetic ntuples (written with uproot)."""
import json
import os
import subprocess
import sys

import awkward as ak
import numpy as np
import uproot

HERE = os.path.dirname(os.path.abspath(__file__))
CONVERT = os.path.join(HERE, '..', 'convert.py')


def write_ntuple(path, first_event, n_processed, broken_mmkk=False):
    extra = {}
    if broken_mmkk:   # mmkk present but without its leg indices: the second channel fails
        extra = {'mmkk_mass': ak.Array([[5.3], [5.4]])}
    with uproot.recreate(path) as f:
        f['bmmg/Events'] = {**extra,
            'evt_run': np.array([1, 1], dtype=np.uint32),
            'evt_lumi': np.array([1, 1], dtype=np.uint32),
            'evt_event': np.array([first_event, first_event + 1], dtype=np.uint64),
            'mu_pt': ak.Array([[5.0, 4.0], [6.0, 3.0]]),
            'pho_pt': ak.Array([[2.0], [3.0]]),
            'dimu_mass': ak.Array([[3.1], [3.0]]),
            'mmg_mass': ak.Array([[5.3], [5.4]]),
            'mmg_fitOK': ak.Array([[1], [1]]),
            'mmg_vtxProb': ak.Array([[0.5], [1e-6]]),
            'mmg_mu1': ak.Array([[0], [0]]),
            'mmg_mu2': ak.Array([[1], [1]]),
            'mmg_pho': ak.Array([[0], [0]]),
            'mmg_dimu': ak.Array([[0], [0]]),
        }
        f['bmmg/RunInfo'] = {'nEvents': np.array([n_processed], dtype=np.uint64),
                             'nEventsStored': np.array([2], dtype=np.uint64),
                             'xsec': np.array([100.]), 'filterEff': np.array([0.5])}


def run(cfg_path, ok=True):
    r = subprocess.run([sys.executable, CONVERT, cfg_path, '--workers', '2'], capture_output=True, text=True)
    assert (r.returncode == 0) == ok, r.stdout + r.stderr
    return r.stdout + r.stderr


def load(out, channel='mmg'):
    from bmmg_ana import io
    return io.load(str(out), 'toy', channel)


def test_convert_incremental_with_bad_file(tmp_path):
    raw, out = tmp_path / 'raw', tmp_path / 'out'
    raw.mkdir()
    write_ntuple(str(raw / 'bmmg_ntuple_1.root'), 100, 1000)
    write_ntuple(str(raw / 'bmmg_ntuple_2.root'), 200, 3000)
    (raw / 'bmmg_ntuple_3.root').write_bytes(b'not a ROOT file')
    cfg = tmp_path / 'convert.yaml'
    cfg.write_text('output: %s\nchannels: [mmg]\npreselection: "fitOK == 1 and vtxProb > 1e-3"\nchunk_files: 1\n'
                   'datasets:\n  toy:\n    files: %s/**/*.root\n    isMC: true\n' % (out, raw))
    log = run(str(cfg))
    assert 'BAD FILE' in log
    rows = load(out)
    assert sorted(rows['evt_event']) == [100, 200]          # one candidate per file passes the preselection
    assert set(rows['mu1_pt']) == {5.0} and set(rows['pho_pt']) == {2.0}
    meta = json.load(open(out / 'toy' / 'meta.json'))
    assert meta['nEvents'] == 4000 and meta['nFiles'] == 2 and meta['xsec'] == 100. and meta['filterEff'] == 0.5
    # second run: nothing new except the bad file (retried); a new file is converted into a new part
    write_ntuple(str(raw / 'bmmg_ntuple_4.root'), 400, 500)
    run(str(cfg))
    rows = load(out)
    assert sorted(rows['evt_event']) == [100, 200, 400]
    assert json.load(open(out / 'toy' / 'meta.json'))['nEvents'] == 4500


def test_failed_file_contributes_nothing_and_settings_are_checked(tmp_path):
    raw, out = tmp_path / 'raw', tmp_path / 'out'
    raw.mkdir()
    write_ntuple(str(raw / 'good.root'), 100, 1000)
    write_ntuple(str(raw / 'broken.root'), 200, 100, broken_mmkk=True)   # mmg converts, mmkk raises
    cfg = tmp_path / 'convert.yaml'
    text = ('output: %s\nchannels: [mmg, mmkk]\npreselection: "fitOK == 1 and vtxProb > 1e-3"\nchunk_files: 5\n'
            'datasets:\n  toy:\n    files: %s/*.root\n    isMC: true\n' % (out, raw))
    cfg.write_text(text)
    assert 'BAD FILE' in run(str(cfg))
    assert sorted(load(out)['evt_event']) == [100]             # no rows of the broken file
    meta = json.load(open(out / 'toy' / 'meta.json'))
    assert meta['nEvents'] == 1000 and meta['nFiles'] == 1      # nor its processed events
    # an orphan part (not in the manifest) is ignored by the reader
    import shutil
    src = sorted((out / 'toy' / 'mmg').glob('part-*.parquet'))[0]
    shutil.copy(src, out / 'toy' / 'mmg' / 'part-00099.parquet')
    assert sorted(load(out)['evt_event']) == [100]
    # different settings must not be mixed with the existing parts
    cfg.write_text(text.replace('vtxProb > 1e-3', 'vtxProb > 1e-2'))
    assert 'different settings' in run(str(cfg), ok=False)
