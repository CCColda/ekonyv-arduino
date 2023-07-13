$vscode_ext_path = "$env:USERPROFILE/.vscode/extensions"
$output_path = "$PSScriptRoot/../libraries.txt"

$path = (Get-ChildItem "$vscode_ext_path/*/assets/platform/*/arduino-cli/arduino-cli.exe" -Recurse)[0].FullName
& $path lib list | % {
	$split = $_ -split "\s+"
	$lib_name = $split[0]
	$lib_version = $split[1]

	if ($lib_name -and $lib_version -and (-not ($lib_version -match "Installed"))) {
		"$lib_name@$lib_version" | Out-File -FilePath $output_path -Append -Encoding UTF8
	}
}
