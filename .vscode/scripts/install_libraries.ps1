$vscode_ext_path = "$env:USERPROFILE/.vscode/extensions"
$input_path = "$PSScriptRoot/../libraries.txt"

$path = (Get-ChildItem "$vscode_ext_path/*/assets/platform/*/arduino-cli/arduino-cli.exe" -Recurse)[0].FullName

Get-Content $input_path | % {
	$lib = $_

	Write-Host "Installing `"$lib`"..."
	& $path lib install $lib
}
