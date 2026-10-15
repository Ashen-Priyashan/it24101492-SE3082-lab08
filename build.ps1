$ErrorActionPreference = 'Stop'

$programs = @(
    @('sum_mpi.c', 'sum_mpi.exe'),
    @('pi_monte_carlo.c', 'pi_monte_carlo.exe'),
    @('pi_any_source.c', 'pi_any_source.exe'),
    @('pi_any_source_bsend.c', 'pi_any_source_bsend.exe'),
    @('message_mismatch.c', 'message_mismatch.exe'),
    @('message2_bsend.c', 'message2_bsend.exe')
)

Push-Location $PSScriptRoot
try {
    foreach ($program in $programs) {
        $source = $program[0]
        $output = $program[1]
        Write-Host "Building $output"
        & mpicc -O2 -Wall -Wextra -std=c11 $source -o $output
        if ($LASTEXITCODE -ne 0) {
            throw "mpicc failed while building $source (exit code $LASTEXITCODE)."
        }
    }
} finally {
    Pop-Location
}
