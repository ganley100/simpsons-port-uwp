# Name:        make_key.ps1
# Description: the key that signs the UWP packages (ports/uwp/uwp.cmake runs it once, when local/ has none): a
#              self-signed code-signing certificate for the Xbox's Developer Mode and Windows' sideloading, its .pfx
#              (no password: CMake reads its thumbprint, MSBuild signs with it) and its .cer (what Device Portal or
#              Windows is given to trust it). The same key each build, so a new package installs over the last one and
#              keeps what the game wrote
# Usage:       powershell -NoProfile -ExecutionPolicy Bypass -File make_key.ps1 -Pfx key.pfx -Cer key.cer -Subject CN=...
param(
	[Parameter(Mandatory = $true)][string]$Pfx,
	[Parameter(Mandatory = $true)][string]$Cer,
	[Parameter(Mandatory = $true)][string]$Subject
)
$ErrorActionPreference = 'Stop'
$cert = New-SelfSignedCertificate -Type Custom -Subject $Subject -KeyUsage DigitalSignature `
	-FriendlyName 'The Simpsons UWP packages (Developer Mode)' -CertStoreLocation 'Cert:\CurrentUser\My' `
	-TextExtension @('2.5.29.37={text}1.3.6.1.5.5.7.3.3', '2.5.29.19={text}') -NotAfter (Get-Date).AddYears(10)
try {
	$folder = Split-Path -Parent $Pfx
	if ($folder) { New-Item -ItemType Directory -Force -Path $folder | Out-Null }
	[System.IO.File]::WriteAllBytes($Pfx, $cert.Export([System.Security.Cryptography.X509Certificates.X509ContentType]::Pfx))
	[System.IO.File]::WriteAllBytes($Cer, $cert.Export([System.Security.Cryptography.X509Certificates.X509ContentType]::Cert))
} finally {
	Remove-Item -Path ('Cert:\CurrentUser\My\' + $cert.Thumbprint)
}
