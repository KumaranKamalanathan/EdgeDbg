$aoApps = Get-AppxPackage
foreach ($oApp in $aoApps) {
  if ($oApp.Name -eq "Microsoft.MicrosoftEdge") {
    $oManifest = Get-AppxPackageManifest $oApp
    foreach ($sId in $oManifest.package.applications.application.id) {
      $oApp.PackageFamilyName + "!" + $sId
    }
  }
}