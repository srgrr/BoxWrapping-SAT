import glob
import os
import subprocess

for filename in glob.glob("instances/*.in"):
  print(filename)
  dataset = os.path.split(filename)[-1].split(".")[0]
  subprocess.call(["./run_and_check.sh", dataset])
