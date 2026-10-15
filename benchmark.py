"""Run MPI programs at several process counts and save CSV/SVG plots."""

import argparse
import csv
import os
import re
import statistics
import subprocess
from pathlib import Path


def run_once(mpiexec, executable, processes, work_size):
    command = [mpiexec, "-n", str(processes), str(executable), str(work_size)]
    result = subprocess.run(command, check=True, capture_output=True, text=True)
    match = re.search(r"TIME_SECONDS=([0-9.eE+-]+)", result.stdout)
    if not match:
        raise RuntimeError(f"No TIME_SECONDS value in output from: {' '.join(command)}")
    return float(match.group(1))


def write_plot(path, title, process_counts, values, y_label, series_label):
    width, height = 760, 440
    left, right, top, bottom = 88, 25, 55, 75
    plot_w, plot_h = width - left - right, height - top - bottom
    max_value = max(values) or 1.0
    max_value *= 1.12
    points = []
    for index, value in enumerate(values):
        x = left + (plot_w * index / max(1, len(values) - 1))
        y = top + plot_h * (1 - value / max_value)
        points.append((x, y, value))

    svg = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}">',
           '<rect width="100%" height="100%" fill="white"/>',
           f'<text x="{width / 2}" y="30" text-anchor="middle" font-family="sans-serif" font-size="20">{title}</text>']
    for tick in range(6):
        value = max_value * tick / 5
        y = top + plot_h * (1 - tick / 5)
        svg.append(f'<line x1="{left}" y1="{y:.1f}" x2="{width-right}" y2="{y:.1f}" stroke="#ddd"/>')
        svg.append(f'<text x="{left-10}" y="{y+4:.1f}" text-anchor="end" font-family="sans-serif" font-size="12">{value:.3g}</text>')
    svg.append(f'<line x1="{left}" y1="{top}" x2="{left}" y2="{height-bottom}" stroke="#333"/>')
    svg.append(f'<line x1="{left}" y1="{height-bottom}" x2="{width-right}" y2="{height-bottom}" stroke="#333"/>')
    for x, count in zip([left + plot_w * i / max(1, len(process_counts)-1) for i in range(len(process_counts))], process_counts):
        svg.append(f'<text x="{x:.1f}" y="{height-bottom+24}" text-anchor="middle" font-family="sans-serif" font-size="12">{count}</text>')
    coords = " ".join(f"{x:.1f},{y:.1f}" for x, y, _ in points)
    svg.append(f'<polyline points="{coords}" fill="none" stroke="#2563eb" stroke-width="3"/>')
    for x, y, value in points:
        svg.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="5" fill="#2563eb"><title>{series_label}: {value:.6f}</title></circle>')
    svg.extend([
        f'<text x="{left+plot_w/2}" y="{height-20}" text-anchor="middle" font-family="sans-serif" font-size="14">Number of processes</text>',
        f'<text x="20" y="{top+plot_h/2}" transform="rotate(-90 20 {top+plot_h/2})" text-anchor="middle" font-family="sans-serif" font-size="14">{y_label}</text>',
        '</svg>'
    ])
    path.write_text("\n".join(svg), encoding="utf-8")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--processes", default="1,2,4", help="comma-separated MPI process counts")
    parser.add_argument("--repeats", type=int, default=3)
    parser.add_argument("--samples", type=int, default=10_000_000)
    parser.add_argument("--output-dir", default="benchmark-results")
    args = parser.parse_args()

    processes = [int(item) for item in args.processes.split(",")]
    if any(item < 1 for item in processes) or args.repeats < 1:
        parser.error("process counts and repeats must be positive")
    mpiexec = os.environ.get("MPIEXEC", "mpiexec")
    root = Path(__file__).resolve().parent
    output = root / args.output_dir
    output.mkdir(parents=True, exist_ok=True)
    programs = {
        "sum": root / "sum_mpi.exe",
        "pi": root / "pi_monte_carlo.exe",
    }
    rows = []
    for name, executable in programs.items():
        if not executable.exists():
            raise FileNotFoundError(f"Build first: {executable} is missing")
        for count in processes:
            times = [run_once(mpiexec, executable, count, args.samples) for _ in range(args.repeats)]
            rows.append({"program": name, "processes": count,
                         "seconds": statistics.median(times), "repeats": args.repeats})

    with (output / "timings.csv").open("w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fieldnames=["program", "processes", "seconds", "repeats"])
        writer.writeheader()
        writer.writerows(rows)

    for name in programs:
        selected = [row for row in rows if row["program"] == name]
        counts = [row["processes"] for row in selected]
        times = [row["seconds"] for row in selected]
        baseline = times[0]
        speedups = [baseline / value if value else 0.0 for value in times]
        write_plot(output / f"{name}-time.svg", f"{name.upper()} runtime", counts, times, "Seconds", "Time")
        write_plot(output / f"{name}-speedup.svg", f"{name.upper()} speedup", counts, speedups, "Speedup", "Speedup")
    print(f"Saved timing table and four plots to {output}")


if __name__ == "__main__":
    main()
