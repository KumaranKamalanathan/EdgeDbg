import os;

gsRecoveryPath = os.path.join(os.getenv("LocalAppData"), "Packages", "Microsoft.MicrosoftEdge_8wekyb3d8bbwe", 
    "AC", "MicrosoftEdge", "User", "Default", "Recovery", "Active");

def fDeleteRecoveryData():
  if not os.path.isdir(gsRecoveryPath):
    return;
  for sFileName in os.listdir(gsRecoveryPath):
    sFilePath = os.path.join(gsRecoveryPath, sFileName);
    os.remove(sFilePath);
