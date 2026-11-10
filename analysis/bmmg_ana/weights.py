"""Normalisation of MC samples to a data luminosity."""


def mc_scale(meta, lumi_pb, xsec_pb=None, filter_eff=None, br_scale=1.0):
    """Weight per MC candidate/event: sigma * filter efficiency * L / N_processed * br_scale.

    meta: meta.json of the MC dataset (bmmg_ana.io.meta). nEvents counts all processed events, also
          with the ntuple skim, so the weight is correct for skimmed ntuples.
    xsec_pb, filter_eff: override the generator values stored in the ntuple (GenRunInfoProduct and
          GenFilterInfo); the generator cross section of B-physics samples is often not the one to use.
    br_scale: e.g. (assumed branching fraction) / (branching fraction used in the generation).
    """
    xsec = xsec_pb if xsec_pb is not None else meta.get('xsec', -1)
    eff = filter_eff if filter_eff is not None else meta.get('filterEff', -1)
    n = meta.get('nEvents', 0)
    if xsec is None or xsec <= 0 or eff is None or eff <= 0 or n <= 0:
        raise ValueError('cannot normalise %s: xsec=%s, filterEff=%s, nEvents=%s (set xsec_pb / filter_eff)'
                         % (meta.get('label'), xsec, eff, n))
    return xsec * eff * lumi_pb / n * br_scale
