"""Read the list of test input files from RadiativeAnalysis/test/test_files.txt.

File format: a header line ending with ':' starts a sample. The lines after it
are paths (glob patterns are allowed) until the next header. Blank lines are
ignored.

    from BsToMuMuGammaAnalysis.RadiativeAnalysis.testFiles import testSamples, testFiles
    files = testFiles(0)   # all files of the first sample, with 'file:' prefix
"""
import glob
import os


def _defaultListPath():
    base = os.environ.get("CMSSW_BASE", "")
    return os.path.join(base, "src", "BsToMuMuGammaAnalysis", "RadiativeAnalysis", "test", "test_files.txt")


def testSamples(listPath=None):
    """Return a list of (description, [path patterns]) in file order."""
    samples = []
    with open(listPath or _defaultListPath()) as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            if line.endswith(":") and not line.startswith("/"):
                samples.append((line[:-1], []))
            elif samples:
                samples[-1][1].append(line)
    return samples


def testFiles(sample=0, listPath=None, maxFiles=-1):
    """Return the files of a sample, prefixed with 'file:'.

    sample: the index of the sample in test_files.txt, or a substring of its description.
    """
    samples = testSamples(listPath)
    if isinstance(sample, int):
        desc, patterns = samples[sample]
    else:
        matches = [s for s in samples if sample.lower() in s[0].lower()]
        if len(matches) != 1:
            raise ValueError("test sample '%s' matches %d entries in test_files.txt" % (sample, len(matches)))
        desc, patterns = matches[0]
    files = []
    for p in patterns:
        expanded = sorted(glob.glob(p)) if any(c in p for c in "*?[") else [p]
        files += expanded
    if maxFiles > 0:
        files = files[:maxFiles]
    return ["file:" + f if f.startswith("/") else f for f in files]


if __name__ == "__main__":
    for i, (desc, patterns) in enumerate(testSamples()):
        print(i, desc, patterns)
