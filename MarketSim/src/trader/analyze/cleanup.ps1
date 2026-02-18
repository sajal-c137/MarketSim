# MarketSim Cleanup Script
# Removes temporary files, build artifacts, and cache

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "MarketSim Cleanup Utility" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

$cleaned = 0

# 1. Python cache
Write-Host "[1/5] Cleaning Python cache..." -ForegroundColor Yellow
Get-ChildItem -Directory -Recurse __pycache__ -ErrorAction SilentlyContinue | ForEach-Object {
    Remove-Item -Recurse -Force $_.FullName
    Write-Host "  ✓ Removed: $($_.FullName)" -ForegroundColor Gray
    $cleaned++
}

# 2. Temporary files
Write-Host "[2/5] Cleaning temporary files..." -ForegroundColor Yellow
Get-ChildItem -Recurse -File | Where-Object { $_.Name -match '\.(tmp|bak|old|swp|swo)$' -or $_.Name -match '^~' } | ForEach-Object {
    Remove-Item -Force $_.FullName
    Write-Host "  ✓ Removed: $($_.Name)" -ForegroundColor Gray
    $cleaned++
}

# 3. Generated chart files in root
Write-Host "[3/5] Cleaning generated output files..." -ForegroundColor Yellow
Get-ChildItem -File *.html,*.png -ErrorAction SilentlyContinue | ForEach-Object {
    Remove-Item -Force $_.FullName
    Write-Host "  ✓ Removed: $($_.Name)" -ForegroundColor Gray
    $cleaned++
}

# 4. Empty log files
Write-Host "[4/5] Cleaning empty log files..." -ForegroundColor Yellow
Get-ChildItem -Recurse -File *.log -ErrorAction SilentlyContinue | Where-Object { $_.Length -eq 0 } | ForEach-Object {
    Remove-Item -Force $_.FullName
    Write-Host "  ✓ Removed: $($_.Name)" -ForegroundColor Gray
    $cleaned++
}

# 5. Old market history (optional - commented out by default)
# Uncomment if you want to clear old market data
# Write-Host "[5/5] Cleaning old market history..." -ForegroundColor Yellow
# if (Test-Path MarketSim\market_history) {
#     Remove-Item -Recurse -Force MarketSim\market_history
#     Write-Host "  ✓ Removed: MarketSim\market_history\" -ForegroundColor Gray
#     $cleaned++
# }

Write-Host "[5/5] Market history cleanup skipped (disabled by default)" -ForegroundColor Gray

Write-Host "`n========================================" -ForegroundColor Green
if ($cleaned -eq 0) {
    Write-Host "✓ Repository is already clean!" -ForegroundColor Green
} else {
    Write-Host "✓ Cleaned $cleaned item(s)" -ForegroundColor Green
}
Write-Host "========================================`n" -ForegroundColor Green
