import sys
import ROOT
ROOT.gROOT.SetBatch(True)

chain = ROOT.TChain("tree")
for f in sys.argv[1:]:
    chain.Add(f)

out = str(sys.argv[1])[:-5]+".pdf"
canvas = ROOT.TCanvas("c", "c", 800, 600)
canvas.Print(out + "[")
for br in chain.GetListOfBranches():
    name = br.GetName()
    n = chain.Draw(f"{name}>>h_{name}", "bsFitSuccessful", "goff")   # vectors are flattened automatically
    h = ROOT.gDirectory.Get(f"h_{name}")
    if n <= 0 or not h:
        continue
    h.SetTitle(name)
    h.Draw("hist")
    canvas.Print(out, f"Title:{name}")
canvas.Print(out + "]")
