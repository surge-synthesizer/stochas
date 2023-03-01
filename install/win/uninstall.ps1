if (Test-Path 'HKLM:\SOFTWARE\WOW6432Node\Microsoft\windows\CurrentVersion\uninstall\{18D03379-6956-4365-BDA3-33D3CBBA991E}_is1' -ErrorAction Ignore) {
  $UNINSTALL = Get-ItemProperty -Path 'HKLM:\SOFTWARE\WOW6432Node\Microsoft\windows\CurrentVersion\uninstall\{18D03379-6956-4365-BDA3-33D3CBBA991E}_is1' | Select-Object -Property UninstallString | Format-Table -HideTableHeaders | Out-String
  $UNINSTALL = $UNINSTALL.Trim()
  $UNINSTALL = $UNINSTALL.Replace("\unins000.exe","")
  $UNINSTALL = $UNINSTALL.Replace("`"","")
  Set-Location $UNINSTALL
  & .\unins000.exe /VERYSILENT
}
elseif (Test-Path 'HKLM:\SOFTWARE\Microsoft\windows\CurrentVersion\uninstall\{18D03379-6956-4365-BDA3-33D3CBBA991E}_is1' -ErrorAction Ignore) {
  $UNINSTALL = Get-ItemProperty -Path 'HKLM:\SOFTWARE\Microsoft\windows\CurrentVersion\uninstall\{18D03379-6956-4365-BDA3-33D3CBBA991E}_is1' | Select-Object -Property UninstallString | Format-Table -HideTableHeaders | Out-String
  $UNINSTALL = $UNINSTALL.Trim()
  $UNINSTALL = $UNINSTALL.Replace("\unins000.exe","")
  $UNINSTALL = $UNINSTALL.Replace("`"","")
  Set-Location $UNINSTALL
  & .\unins000.exe /VERYSILENT
}
