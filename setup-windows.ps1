#!/usr/bin/env pwsh
<#
.SYNOPSIS
	Setup automático para FaseOrbital en Windows
.DESCRIPTION
	Script que abre Visual Studio y prepara el proyecto
.EXAMPLE
	.\setup-windows.ps1
#>

# Configuración
$projectName = "FaseOrbital"
$projectFile = "FaseOrbital\FaseOrbital.vcxproj"
$guideFile = "PROFESOR_WINDOWS_RAPIDO.md"

Write-Host "`n" -ForegroundColor White
Write-Host "╔════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║       FaseOrbital - Setup Automático Windows           ║" -ForegroundColor Cyan
Write-Host "╚════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host "`n"

# Verificar que estamos en la carpeta correcta
if (-not (Test-Path $projectFile)) {
	Write-Host "❌ Error: No se encuentra $projectFile" -ForegroundColor Red
	Write-Host "   Asegúrate de ejecutar este script desde la raíz del proyecto" -ForegroundColor Yellow
	Write-Host "   Carpeta actual: $(Get-Location)" -ForegroundColor White
	Write-Host ""
	Read-Host "Presiona Enter para salir"
	exit 1
}

Write-Host "✅ Proyecto encontrado en: $(Get-Location)" -ForegroundColor Green
Write-Host ""

# Paso 1: Verificar Visual Studio
Write-Host "📋 Paso 1/3: Buscando Visual Studio..." -ForegroundColor Yellow

$vsPath = $null
$vsPaths = @(
	"C:\Program Files\Microsoft Visual Studio\2026\Community\Common7\IDE\devenv.exe",
	"C:\Program Files\Microsoft Visual Studio\2025\Community\Common7\IDE\devenv.exe",
	"C:\Program Files\Microsoft Visual Studio\2024\Community\Common7\IDE\devenv.exe",
	"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\devenv.exe"
)

foreach ($path in $vsPaths) {
	if (Test-Path $path) {
		$vsPath = $path
		break
	}
}

if (-not $vsPath) {
	Write-Host "❌ Visual Studio no encontrado" -ForegroundColor Red
	Write-Host ""
	Write-Host "   Descarga Visual Studio Community desde:" -ForegroundColor Yellow
	Write-Host "   https://visualstudio.microsoft.com/vs/community/" -ForegroundColor Cyan
	Write-Host ""
	Read-Host "Presiona Enter para abrir el navegador"
	Start-Process "https://visualstudio.microsoft.com/vs/community/"
	exit 1
}

Write-Host "✅ Visual Studio encontrado" -ForegroundColor Green
Write-Host ""

# Paso 2: Abrir proyecto
Write-Host "📋 Paso 2/3: Abriendo proyecto en Visual Studio..." -ForegroundColor Yellow

$projectFullPath = Resolve-Path $projectFile
Write-Host "   Archivo: $projectFullPath" -ForegroundColor White

# Abre Visual Studio con el proyecto
& $vsPath $projectFullPath

Write-Host "✅ Visual Studio abierto" -ForegroundColor Green
Write-Host ""

# Paso 3: Mostrar instrucciones
Write-Host "📋 Paso 3/3: Próximos pasos" -ForegroundColor Yellow
Write-Host ""
Write-Host "   En Visual Studio, debes:" -ForegroundColor White
Write-Host "   1️⃣  Ir a: Tools → NuGet Package Manager → Package Manager Console" -ForegroundColor Cyan
Write-Host "   2️⃣  Escribe: $('Install-Package Allegro')" -ForegroundColor Cyan
Write-Host "   3️⃣  Presiona Enter y espera a que instale" -ForegroundColor Cyan
Write-Host "   4️⃣  Cuando termine, presiona Ctrl+Shift+B para compilar" -ForegroundColor Cyan
Write-Host "   5️⃣  Luego presiona F5 para ejecutar el juego" -ForegroundColor Cyan
Write-Host ""

# Mensaje final
Write-Host "╔════════════════════════════════════════════════════════╗" -ForegroundColor Green
Write-Host "║              ¡Casi listo!                              ║" -ForegroundColor Green
Write-Host "║                                                        ║" -ForegroundColor Green
Write-Host "║  Sigue los pasos anteriores en Visual Studio           ║" -ForegroundColor Green
Write-Host "║  Si necesitas ayuda, lee: $guideFile" -ForegroundColor Green
Write-Host "╚════════════════════════════════════════════════════════╝" -ForegroundColor Green
Write-Host ""
