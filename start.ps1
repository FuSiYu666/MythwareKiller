<#
    MythwareKiller - Removal Tool
    Author: FSY
    Homepage: https://github.com/FuSiYu666/MythwareKiller
#>

# ===== CONFIGURATION =====
$global:Config = @{
    # Basic information
    Author = "FSY"
    ToolName = "MythwareKiller"
    Homepage = "https://github.com/FuSiYu666/MythwareKiller"
    TroubleshootURL = "https://github.com/FuSiYu666/MythwareKiller/issues"
    
    # Download settings
    DownloadDir = "$env:USERPROFILE\!MythwareKiller"
    MainExecutable = "main.exe"
    Version = "2.0.0"  # 定义版本号
    
    # Archive download options with multiple mirrors
    ArchiveOptions = @{
        "ZIP" = @(
            'https://myth.fsyfsyfsy.dpdns.org/MythwareKiller-2.0.0.zip',
            'https://myth.fsy.qzz.io/MythwareKiller-2.0.0.zip',
            'https://github.com/FuSiYu666/MythwareKiller/releases/latest/download/MythwareKiller-2.0.0.zip'
        );
        "7Z" = @(
            'https://myth.fsyfsyfsy.dpdns.org/MythwareKiller-2.0.0.7z',
            'https://myth.fsy.qzz.io/MythwareKiller-2.0.0.7z', 
            'https://github.com/FuSiYu666/MythwareKiller/releases/latest/download/MythwareKiller-2.0.0.7z'
        )
    }
    
    # Network settings
    PingAttempts = 1
    SpeedUpdateInterval = 0.5 # seconds
    
    # Display colors
    Colors = @{
        Success = "Green"
        Error = "Red"
        Warning = "Yellow"
        Info = "Cyan"
        Debug = "DarkGray"
        Highlight = "White"
    }
}

# ===== MAIN SCRIPT =====
if (-not $args) {
    Write-Host ""
    Write-Host "$($global:Config.ToolName) - Removal Tool by $($global:Config.Author)" -ForegroundColor $global:Config.Colors.Info
    Write-Host "Need help? Check the homepage: " -NoNewline
    Write-Host $global:Config.Homepage -ForegroundColor $global:Config.Colors.Success
    Write-Host ""
}

& {
    $psv = (Get-Host).Version.Major

    # Check system requirements
    if ($ExecutionContext.SessionState.LanguageMode.value__ -ne 0) {
        $ExecutionContext.SessionState.LanguageMode
        Write-Host "PowerShell is not running in Full Language Mode."
        Write-Host "Help - $($global:Config.TroubleshootURL)" -ForegroundColor $global:Config.Colors.Highlight -BackgroundColor Blue
        return
    }

    try {
        [void][System.AppDomain]::CurrentDomain.GetAssemblies(); [void][System.Math]::Sqrt(144)
    }
    catch {
        Write-Host "Error: $($_.Exception.Message)" -ForegroundColor $global:Config.Colors.Error
        Write-Host "Powershell failed to load .NET command."
        Write-Host "Help - $($global:Config.TroubleshootURL)" -ForegroundColor $global:Config.Colors.Highlight -BackgroundColor Blue
        return
    }

    function Check3rdAV {
        $cmd = if ($psv -ge 3) { 'Get-CimInstance' } else { 'Get-WmiObject' }
        $avList = & $cmd -Namespace root\SecurityCenter2 -Class AntiVirusProduct | Where-Object { $_.displayName -notlike '*windows*' } | Select-Object -ExpandProperty displayName

        if ($avList) {
            Write-Host '3rd party Antivirus might be blocking the script - ' -ForegroundColor $global:Config.Colors.Highlight -BackgroundColor Blue -NoNewline
            Write-Host " $($avList -join ', ')" -ForegroundColor DarkRed -BackgroundColor White
        }
    }

    function CheckFile {
        param ([string]$FilePath)
        if (-not (Test-Path $FilePath)) {
            Check3rdAV
            Write-Host "Failed to create file in target folder, aborting!" -ForegroundColor $global:Config.Colors.Error
            Write-Host "Help - $($global:Config.TroubleshootURL)" -ForegroundColor $global:Config.Colors.Highlight -BackgroundColor Blue
            throw
        }
    }


    try { [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12 } catch {}

    # Create download directory if it doesn't exist
    if (-not (Test-Path $global:Config.DownloadDir)) {
        New-Item -ItemType Directory -Path $global:Config.DownloadDir -Force | Out-Null
        Write-Host "Created download directory: $($global:Config.DownloadDir)" -ForegroundColor $global:Config.Colors.Debug
    }

    # Function to test mirror latency
    function Test-MirrorLatency {
        param ([string]$Url)
        
        try {
            $uri = [System.Uri]$Url
            $hostName = $uri.Host
            
            Write-Host "Pinging $hostName..." -NoNewline -ForegroundColor $global:Config.Colors.Debug
            
            # Test ping (4 attempts)
            $pingResults = Test-Connection -ComputerName $hostName -Count $global:Config.PingAttempts -ErrorAction Stop
            
            $avgLatency = ($pingResults | Measure-Object -Property ResponseTime -Average).Average
            Write-Host " Average latency: $avgLatency ms" -ForegroundColor Gray
            
            return [PSCustomObject]@{
                Url = $Url
                Host = $hostName
                Latency = $avgLatency
            }
        }
        catch {
            Write-Host " Failed (unreachable)" -ForegroundColor $global:Config.Colors.Error
            return [PSCustomObject]@{
                Url = $Url
                Host = $hostName
                Latency = [double]::MaxValue
            }
        }
    }

    # Function to select fastest mirror
    function Get-FastestMirror {
        param ([string[]]$Urls)
        
        Write-Host "`nTesting mirror speeds..." -ForegroundColor $global:Config.Colors.Info
        
        $mirrorTests = @()
        foreach ($url in $Urls) {
            $mirrorTests += Test-MirrorLatency -Url $url
        }
        
        # Sort by latency and select the fastest available mirror
        $fastestMirror = $mirrorTests | Sort-Object Latency | Select-Object -First 1
        
        if ($fastestMirror.Latency -eq [double]::MaxValue) {
            Write-Host "All mirrors are unreachable!" -ForegroundColor $global:Config.Colors.Error
            return $null
        }
        
        Write-Host "`nSelected fastest mirror: $($fastestMirror.Host) ($($fastestMirror.Latency) ms)" -ForegroundColor $global:Config.Colors.Success
        return $fastestMirror.Url
    }

    # Function to format file size
    function Format-FileSize {
        param ([long]$Size)
        if ($Size -gt 1GB) { return "{0:N2} GB" -f ($Size / 1GB) }
        elseif ($Size -gt 1MB) { return "{0:N2} MB" -f ($Size / 1MB) }
        elseif ($Size -gt 1KB) { return "{0:N2} KB" -f ($Size / 1KB) }
        else { return "$Size B" }
    }

    # Function to format speed
    function Format-Speed {
        param ([long]$BytesPerSecond)
        if ($BytesPerSecond -gt 1GB) { return "{0:N2} GB/s" -f ($BytesPerSecond / 1GB) }
        elseif ($BytesPerSecond -gt 1MB) { return "{0:N2} MB/s" -f ($BytesPerSecond / 1MB) }
        elseif ($BytesPerSecond -gt 1KB) { return "{0:N2} KB/s" -f ($BytesPerSecond / 1KB) }
        else { return "$BytesPerSecond B/s" }
    }

    # Function to download file with progress
    function Download-FileWithProgress {
        param (
            [string]$FileName,
            [string[]]$URLs,
            [string]$DownloadDir
        )
        
        $filePath = Join-Path -Path $DownloadDir -ChildPath $FileName
        $errors = @()
        
        # Select fastest mirror first
        $fastestUrl = Get-FastestMirror -Urls $URLs
        if (-not $fastestUrl) {
            Write-Host "No available mirrors for $FileName" -ForegroundColor $global:Config.Colors.Error
            return $null
        }
        
        # Display file info
        Write-Host "`nDownloading: $FileName" -ForegroundColor $global:Config.Colors.Info
        Write-Host "Selected mirror: $fastestUrl" -ForegroundColor $global:Config.Colors.Warning
        
        try {
            if ($psv -ge 3) {
                # For PowerShell 3.0 and above
                $request = [System.Net.HttpWebRequest]::Create($fastestUrl)
                $response = $request.GetResponse()
                $totalLength = [System.Math]::Floor($response.ContentLength / 1KB)
                $responseStream = $response.GetResponseStream()
                
                $fileStream = New-Object System.IO.FileStream($filePath, [System.IO.FileMode]::Create)
                $buffer = New-Object byte[] 10KB
                $count = $responseStream.Read($buffer, 0, $buffer.Length)
                $downloadedBytes = $count
                $startTime = Get-Date
                $prevTime = $startTime
                $prevBytes = 0
                
                while ($count -gt 0) {
                    $fileStream.Write($buffer, 0, $count)
                    $count = $responseStream.Read($buffer, 0, $buffer.Length)
                    $downloadedBytes += $count
                    
                    $currentTime = Get-Date
                    $timeElapsed = ($currentTime - $startTime).TotalSeconds
                    $timeDelta = ($currentTime - $prevTime).TotalSeconds
                    
                    if ($timeDelta -ge $global:Config.SpeedUpdateInterval) {
                        $bytesDelta = $downloadedBytes - $prevBytes
                        $speed = $bytesDelta / $timeDelta
                        $prevBytes = $downloadedBytes
                        $prevTime = $currentTime
                        
                        $percentComplete = ($downloadedBytes / $response.ContentLength) * 100
                        $progressParams = @{
                            Activity = "Downloading $FileName"
                            Status = ("{0:N2}% complete ({1} of {2})" -f `
                                $percentComplete, `
                                (Format-FileSize $downloadedBytes), `
                                (Format-FileSize $response.ContentLength))
                            CurrentOperation = "Speed: $(Format-Speed $speed)"
                            PercentComplete = $percentComplete
                        }
                        Write-Progress @progressParams
                    }
                }
                
                $fileStream.Close()
                $responseStream.Close()
                $response.Close()
                
                Write-Progress -Activity "Downloading $FileName" -Completed
                Write-Host "Download completed successfully!" -ForegroundColor $global:Config.Colors.Success
                return $filePath
            }
            else {
                # For PowerShell 2.0
                $webClient = New-Object System.Net.WebClient
                $webClient.DownloadFile($fastestUrl, $filePath)
                
                if (Test-Path $filePath) {
                    Write-Host "Download completed successfully!" -ForegroundColor $global:Config.Colors.Success
                    return $filePath
                }
            }
        }
        catch {
            $errors += $_
            Write-Host "Failed to download from $fastestUrl" -ForegroundColor $global:Config.Colors.Error
            Write-Host "Error: $($_.Exception.Message)" -ForegroundColor DarkRed
            
            # Fallback to other mirrors if fastest fails
            $otherUrls = $URLs | Where-Object { $_ -ne $fastestUrl }
            if ($otherUrls.Count -gt 0) {
                Write-Host "Attempting fallback to other mirrors..." -ForegroundColor $global:Config.Colors.Warning
                foreach ($url in $otherUrls) {
                    try {
                        Write-Host "Trying mirror: $url" -ForegroundColor DarkYellow
                        if ($psv -ge 3) {
                            Invoke-WebRequest -Uri $url -OutFile $filePath -UseBasicParsing
                        }
                        else {
                            $webClient = New-Object System.Net.WebClient
                            $webClient.DownloadFile($url, $filePath)
                        }
                        
                        if (Test-Path $filePath) {
                            Write-Host "Fallback download succeeded!" -ForegroundColor $global:Config.Colors.Success
                            return $filePath
                        }
                    }
                    catch {
                        $errors += $_
                        Write-Host "Fallback attempt failed: $($_.Exception.Message)" -ForegroundColor DarkRed
                    }
                }
            }
            
            Check3rdAV
            foreach ($err in $errors) {
                Write-Host "Error: $($err.Exception.Message)" -ForegroundColor $global:Config.Colors.Error
            }
            Write-Host "Failed to download $FileName from any available source" -ForegroundColor $global:Config.Colors.Error
            Write-Host "Please report this issue at: $($global:Config.TroubleshootURL)" -ForegroundColor $global:Config.Colors.Warning
            return $null
        }
    }

    # Function to extract archive
    function Expand-ArchiveFile {
        param (
            [string]$FilePath,
            [string]$Destination
        )
        
        $extension = [System.IO.Path]::GetExtension($FilePath).ToLower()
        
        try {
            Write-Host "`nExtracting $FilePath to $Destination..." -ForegroundColor $global:Config.Colors.Info
            
            # Create destination if it doesn't exist
            if (-not (Test-Path $Destination)) {
                New-Item -ItemType Directory -Path $Destination -Force | Out-Null
            }
            
            # Use appropriate extraction method based on file extension
            switch ($extension) {
                '.zip' {
                    if ($psv -ge 5) {
                        Expand-Archive -Path $FilePath -DestinationPath $Destination -Force
                    }
                    else {
                        # Fallback for PowerShell < 5
                        $shell = New-Object -ComObject Shell.Application
                        $zipFile = $shell.NameSpace($FilePath)
                        $destinationFolder = $shell.NameSpace($Destination)
                        $destinationFolder.CopyHere($zipFile.Items(), 0x14) # 0x14 = 16 (overwrite) + 4 (no progress UI)
                    }
                }
                '.7z' {
                    # Check if 7-Zip is available
                    $7zipPath = "$env:ProgramFiles\7-Zip\7z.exe"
                    if (-not (Test-Path $7zipPath)) {
                        $7zipPath = "$env:ProgramFiles (x86)\7-Zip\7z.exe"
                    }
                    
                    if (Test-Path $7zipPath) {
                        $args = "x `"$FilePath`" -o`"$Destination`" -y"
                        Start-Process -FilePath $7zipPath -ArgumentList $args -Wait -WindowStyle Hidden
                    }
                    else {
                        throw "7-Zip is required to extract .7z files. Please install 7-Zip and try again."
                    }
                }
                default {
                    throw "Unsupported archive format: $extension"
                }
            }
            
            Write-Host "Extraction completed successfully!" -ForegroundColor $global:Config.Colors.Success
            return $true
        }
        catch {
            Write-Host "Failed to extract archive: $($_.Exception.Message)" -ForegroundColor $global:Config.Colors.Error
            return $false
        }
    }

    # Let user select archive format
    $selectedFormat = "ZIP"  # 自动选择ZIP格式
    $archiveUrls = $global:Config.ArchiveOptions[$selectedFormat]
    $archiveName = "$($global:Config.ToolName)-$($global:Config.Version).$($selectedFormat.ToLower())"
    $archivePath = Join-Path -Path $global:Config.DownloadDir -ChildPath $archiveName

    # Download the selected archive
    $downloadedFile = Download-FileWithProgress -FileName $archiveName -URLs $archiveUrls -DownloadDir $global:Config.DownloadDir
    if (-not $downloadedFile) {
        Write-Host "Archive download failed. Aborting." -ForegroundColor $global:Config.Colors.Error
        exit 1
    }

    # Extract the archive
    $extractionSuccess = Expand-ArchiveFile -FilePath $archivePath -Destination $global:Config.DownloadDir
    if (-not $extractionSuccess) {
        Write-Host "Archive extraction failed. Aborting." -ForegroundColor $global:Config.Colors.Error
        exit 1
    }

    # Run the main executable after extraction
    $mainExecutable = Join-Path -Path $global:Config.DownloadDir -ChildPath "$($global:Config.ToolName)-$($global:Config.Version)" | Join-Path -ChildPath $global:Config.MainExecutable
    if (Test-Path $mainExecutable) {
        Write-Host "`nStarting $($global:Config.ToolName)..." -ForegroundColor $global:Config.Colors.Info
        Write-Host "For more information, visit: $($global:Config.Homepage)" -ForegroundColor $global:Config.Colors.Debug
        try {
            Start-Process -FilePath $mainExecutable -Wait
        }
        catch {
            Write-Host "Failed to start the executable: $($_.Exception.Message)" -ForegroundColor $global:Config.Colors.Error
            Write-Host "Try running it manually from: $mainExecutable" -ForegroundColor $global:Config.Colors.Warning
            Write-Host "If problems persist, please report at: $($global:Config.TroubleshootURL)" -ForegroundColor $global:Config.Colors.Warning
        }
    }
    else {
        Write-Host "Main executable not found in extracted files" -ForegroundColor $global:Config.Colors.Error
        Write-Host "Expected path: $mainExecutable" -ForegroundColor $global:Config.Colors.Debug
        Write-Host "Please report this issue at: $($global:Config.TroubleshootURL)" -ForegroundColor $global:Config.Colors.Warning
    }
} @args
