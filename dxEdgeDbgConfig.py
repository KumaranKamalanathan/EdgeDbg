import os;
sBaseDir = os.path.dirname(__file__);

try:
  from dxConfig import dxConfig;
except:
  dxConfig = {};

# Add EdgeDbg group if it doesn't exist.
dxEdgeDbgConfig = dxConfig.setdefault("EdgeDbg", {});
# Add default values where no values have been supplied:
for (sName, xValue) in {
  "sEdgeDbgBinaryPath_x86": os.path.join(sBaseDir, "bin", "EdgeDbg_x86.exe"),
  "sEdgeDbgBinaryPath_x64": os.path.join(sBaseDir, "bin", "EdgeDbg_x64.exe"),
}.items():
  if sName not in dxEdgeDbgConfig:
    dxEdgeDbgConfig[sName] = xValue;
