"""Read the test input samples from RadiativeAnalysis/test/test_files.yaml.

Each sample is keyed by a short name and has the attributes 'isMC' (bool),
'dataFormat' ('AOD' / 'MINIAOD'), 'files' (paths, glob patterns allowed) and an
optional 'description'.

    from BsToMuMuGammaAnalysis.RadiativeAnalysis.testFiles import testSample, testFiles
    sample = testSample('bsmmg_aod')   # dict: name, description, isMC, dataFormat, files (patterns)
    files = testFiles('bsmmg_aod')     # the expanded files, with 'file:' prefix
"""
import glob
import os

DATA_FORMATS = ('AOD', 'MINIAOD')


def _defaultListPath():
    base = os.environ.get("CMSSW_BASE", "")
    return os.path.join(base, "src", "BsToMuMuGammaAnalysis", "RadiativeAnalysis", "test", "test_files.yaml")


def testSamples(listPath=None):
    """Return {name: sample dict} in file order; every sample is checked for the required attributes."""
    import yaml
    path = listPath or _defaultListPath()
    with open(path) as f:
        content = yaml.safe_load(f) or {}
    samples = {}
    for name, s in content.items():
        if not isinstance(s, dict):
            raise ValueError("%s: sample '%s' is not a mapping" % (path, name))
        missing = {'isMC', 'dataFormat', 'files'} - set(s)
        if missing:
            raise ValueError("%s: sample '%s' misses %s" % (path, name, ', '.join(sorted(missing))))
        unknown = set(s) - {'isMC', 'dataFormat', 'files', 'description'}
        if unknown:
            raise ValueError("%s: sample '%s' has unknown keys %s" % (path, name, ', '.join(sorted(unknown))))
        if not isinstance(s['isMC'], bool):
            raise ValueError("%s: sample '%s': isMC must be true or false" % (path, name))
        dataFormat = str(s['dataFormat']).upper()
        if dataFormat not in DATA_FORMATS:
            raise ValueError("%s: sample '%s': dataFormat must be AOD or MINIAOD" % (path, name))
        files = s['files'] if isinstance(s['files'], list) else [s['files']]
        samples[name] = dict(name=name, description=s.get('description', ''), isMC=s['isMC'],
                             dataFormat=dataFormat, files=[str(p) for p in files])
    return samples


def testSample(name, listPath=None):
    """Return the sample dict of the given name."""
    samples = testSamples(listPath)
    if name not in samples:
        raise ValueError("unknown test sample '%s'; available: %s" % (name, ', '.join(samples)))
    return samples[name]


def testFiles(name, listPath=None, maxFiles=-1):
    """Return the files of a sample (glob patterns expanded), prefixed with 'file:'."""
    files = []
    for p in testSample(name, listPath)['files']:
        files += sorted(glob.glob(p)) if any(c in p for c in "*?[") else [p]
    if maxFiles > 0:
        files = files[:maxFiles]
    return ["file:" + f if f.startswith("/") else f for f in files]


def datasetFiles(dataset):
    """Return the files of a dataset, prefixed with the global xrootd redirector."""
    from subprocess import check_output
    cmd = ["dasgoclient", "-query", "file dataset=%s" % dataset]
    output = check_output(cmd, universal_newlines=True)
    return ["root://cms-xrd-global.cern.ch/" + f for f in output.splitlines() if f.strip()]


if __name__ == "__main__":
    for name, s in testSamples().items():
        print("%-20s %-4s %-7s %s" % (name, 'MC' if s['isMC'] else 'data', s['dataFormat'], s['description']))
        for p in s['files']:
            print("%20s %s" % ('', p))
