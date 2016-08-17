import os, sys;
# The CWD may not be this script's folder; make sure it looks there for modules first:
sBaseFolderPath = os.path.dirname(__file__);
for sPath in [sBaseFolderPath] + [os.path.join(sBaseFolderPath, x) for x in ["modules"]]:
  if sPath not in sys.path:
    sys.path.insert(0, sPath);

from FileSystem import FileSystem;

sRecoveryPath = FileSystem.fsPath(os.getenv("LocalAppData"), "Packages", "Microsoft.MicrosoftEdge_8wekyb3d8bbwe", 
    "AC", "MicrosoftEdge", "User", "Default", "Recovery", "Active");

def fDeleteRecoveryData():
  FileSystem.fbDeleteChildrenFromFolder(sRecoveryPath);
