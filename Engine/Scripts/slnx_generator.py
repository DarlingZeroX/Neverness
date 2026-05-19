#!/usr/bin/env python3
"""Generate a .slnx solution at the repo root listing all C# projects under Engine/Source/Managed."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path
from xml.dom import minidom
from xml.etree import ElementTree as ET

MANAGED_REL = Path("Engine") / "Source" / "Managed"
DEFAULT_SLNX_NAME = "Neverness.slnx"

PLATFORM_CONFIGS = ("Any CPU", "x64", "x86")


def repo_root_from_script() -> Path:
    return Path(__file__).resolve().parents[2]


def discover_csproj_files(managed_root: Path) -> list[Path]:
    if not managed_root.is_dir():
        raise FileNotFoundError(f"Managed sources directory not found: {managed_root}")

    projects: list[Path] = []
    for path in managed_root.rglob("*.csproj"):
        parts = {part.lower() for part in path.parts}
        if parts & {"obj", "bin"}:
            continue
        projects.append(path)

    return sorted(projects, key=lambda p: p.as_posix().lower())


def to_repo_relative_posix(project: Path, repo_root: Path) -> str:
    return project.relative_to(repo_root).as_posix()


def build_solution_element(projects: list[str]) -> ET.Element:
    solution = ET.Element("Solution")

    configurations = ET.SubElement(solution, "Configurations")
    for platform in PLATFORM_CONFIGS:
        ET.SubElement(configurations, "Platform", Name=platform)

    for project_path in projects:
        ET.SubElement(solution, "Project", Path=project_path)

    return solution


def serialize_solution(solution: ET.Element) -> str:
    rough = ET.tostring(solution, encoding="unicode")
    parsed = minidom.parseString(rough)
    lines = parsed.toprettyxml(indent="  ", encoding=None).splitlines()
    # Drop minidom's XML declaration; .slnx files are typically declaration-free.
    body = [line for line in lines if line.strip() and not line.strip().startswith("<?xml")]
    return "\n".join(body) + "\n"


def write_slnx(
    repo_root: Path,
    managed_root: Path,
    output_path: Path,
    *,
    dry_run: bool,
) -> int:
    projects_on_disk = discover_csproj_files(managed_root)
    if not projects_on_disk:
        print(f"No .csproj files found under {managed_root}", file=sys.stderr)
        return 1

    project_paths = [to_repo_relative_posix(p, repo_root) for p in projects_on_disk]
    content = serialize_solution(build_solution_element(project_paths))

    if dry_run:
        print(content, end="")
        print(
            f"# dry-run: would write {len(project_paths)} project(s) to {output_path}",
            file=sys.stderr,
        )
        return 0

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(content, encoding="utf-8", newline="\n")
    print(f"Wrote {output_path} ({len(project_paths)} projects)")
    return 0


def parse_args() -> argparse.Namespace:
    default_repo = repo_root_from_script()
    parser = argparse.ArgumentParser(
        description="Generate a .slnx file for all C# projects under Engine/Source/Managed.",
    )
    parser.add_argument(
        "--repo-root",
        type=Path,
        default=default_repo,
        help=f"Neverness repository root (default: {default_repo})",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=None,
        help=f"Output .slnx path (default: <repo-root>/{DEFAULT_SLNX_NAME})",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Print generated XML to stdout instead of writing a file",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    repo_root = args.repo_root.resolve()
    managed_root = repo_root / MANAGED_REL
    output_path = (
        args.output.resolve()
        if args.output is not None
        else repo_root / DEFAULT_SLNX_NAME
    )

    try:
        return write_slnx(repo_root, managed_root, output_path, dry_run=args.dry_run)
    except FileNotFoundError as exc:
        print(exc, file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
