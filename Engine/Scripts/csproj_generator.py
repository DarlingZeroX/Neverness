#!/usr/bin/env python3
r"""Generate .csproj files for modules under Engine/Source/Managed.

Default layout follows NervernessEditor.csproj. Edit DEFAULTS / MODULE_OVERRIDES
to change shared or per-module settings, then re-run this script.

Usage (from repo root)::

    python Engine/Scripts/csproj_generator.py --dry-run
    python Engine/Scripts/csproj_generator.py --force
    python Engine/Scripts/csproj_generator.py --module Neverness.Editor.Framework --force
"""

from __future__ import annotations

import argparse
import sys
from dataclasses import dataclass, field
from pathlib import Path

MANAGED_REL = Path("Engine") / "Source" / "Managed"

SKIP_DIR_NAMES = frozenset(
    {
        "obj",
        "bin",
        "Build",
        "Generated",
        "native",
        "Docs",
        "Properties",
    }
)

SKIP_MODULE_NAMES = frozenset(
    {
        "Neverness.Editor.ImGui",
        "Neverness.Editor.ImGuiNodeEditor",
        "Neverness.Editor.ImGuizmo",
    }
)

# NervernessEditor.csproj baseline (also used for Neverness.Editor.* libraries).
EDITOR_DEFAULTS: dict[str, str | bool] = {
    "target_framework": "net10.0",
    "implicit_usings": "enable",
    "nullable": "enable",
    "enable_native_code_debugging": True,
    "debug_type": "portable",
    "generate_assembly_info": False,
    "generate_target_framework_attribute": False,
    "append_target_framework_to_output_path": False,
    "append_runtime_identifier_to_output_path": False,
}

# Property names emitted by render_editor_csproj (extras go after the standard block).
EDITOR_STANDARD_KEYS = frozenset(
    {
        "AssemblyName",
        "OutputType",
        "TargetFramework",
        "ImplicitUsings",
        "Nullable",
        "EnableNativeCodeDebugging",
        "DebugType",
        "BaseOutputPath",
        "OutputPath",
        "IntermediateOutputPath",
        "GenerateAssemblyInfo",
        "GenerateTargetFrameworkAttribute",
        "AppendTargetFrameworkToOutputPath",
        "AppendRuntimeIdentifierToOutputPath",
    }
)

MODULE_OVERRIDES: dict[str, dict] = {
    "NervernessEditor": {
        "assembly_name": "NevernessEditor",
        "intermediate_name": "NevernessEditor",
        "template": "editor_exe",
    },
    "Neverness.Editor.Framework": {
        "extra_properties": {
            "AllowUnsafeBlocks": "true",
            "RollForward": "LatestMinor",
            "RootNamespace": "VisionGal.Managed.Editor",
        },
        "project_references": [
            r"..\..\Runtime\Neverness.Runtime.Object\Neverness.Runtime.Object.csproj",
        ],
    },
    "Tests": {
        "project_file": "NevernessRuntimeManaged-Foundation.Tests.csproj",
        "assembly_name": "Neverness.Runtime.Foundation.Tests",
        "intermediate_name": "NevernessRuntimeManaged-Foundation.Tests",
        "template": "runtime",
        "extra_properties": {
            "IsPackable": "false",
            "AllowUnsafeBlocks": "true",
            "RootNamespace": "Neverness.Managed.Foundation.Tests",
        },
        "package_references": [
            ("Microsoft.NET.Test.Sdk", "17.14.1"),
            ("xunit", "2.9.3"),
            ("xunit.runner.visualstudio", "3.1.4"),
        ],
        "project_references": [
            r"..\Neverness.Runtime.Assets\Neverness.Runtime.Assets.csproj",
            r"..\Neverness.Runtime.Object\Neverness.Runtime.Object.csproj",
            r"..\Neverness.Runtime.Reflection\Neverness.Runtime.Reflection.csproj",
            r"..\Neverness.Runtime.RuntimeLoop\Neverness.Runtime.RuntimeLoop.csproj",
            r"..\Neverness.Runtime.Scene\Neverness.Runtime.Scene.csproj",
            r"..\Neverness.Runtime.Serialization\Neverness.Runtime.Serialization.csproj",
            r"..\Neverness.Runtime.Gameplay\Neverness.Runtime.Gameplay.csproj",
        ],
    },
}

RUNTIME_LIBRARY_DEFAULTS: dict[str, str | bool] = {
    "target_framework": "net10.0",
    "implicit_usings": "enable",
    "nullable": "enable",
    "allow_unsafe_blocks": True,
    "roll_forward": "LatestMinor",
    "base_output_path": "$(SolutionDir)Build\\bin\\",
    "output_path": "$(BaseOutputPath)$(Configuration)\\",
    "append_target_framework_to_output_path": False,
    "append_runtime_identifier_to_output_path": False,
}


@dataclass
class ProjectSpec:
    module_dir: Path
    module_name: str
    project_file: str
    assembly_name: str
    intermediate_name: str
    template: str
    target_framework: str = "net10.0"
    properties: dict[str, str | bool] = field(default_factory=dict)
    package_references: list[tuple[str, str]] = field(default_factory=list)
    project_references: list[str] = field(default_factory=list)


def repo_root_from_script() -> Path:
    return Path(__file__).resolve().parents[2]


def template_kind(module_name: str, override: dict) -> str:
    if "template" in override:
        return str(override["template"])
    if module_name == "NervernessEditor":
        return "editor_exe"
    if module_name.startswith("Neverness.Editor."):
        return "editor_library"
    if module_name.startswith("Neverness.Runtime.") or module_name == "Tests":
        return "runtime"
    if module_name.startswith("Nerverness"):
        return "editor_exe"
    return "runtime"


def has_csharp_sources(module_dir: Path) -> bool:
    for path in module_dir.rglob("*.cs"):
        if any(part in SKIP_DIR_NAMES for part in path.parts):
            continue
        return True
    return False


def is_managed_module_name(name: str) -> bool:
    if name in MODULE_OVERRIDES:
        return True
    return name.startswith("Neverness.") or name.startswith("Nerverness")


def discover_module_dirs(managed_root: Path) -> list[Path]:
    modules: list[Path] = []
    for path in sorted(managed_root.rglob("*")):
        if not path.is_dir():
            continue
        if any(part in SKIP_DIR_NAMES for part in path.parts):
            continue
        if path.name in SKIP_MODULE_NAMES:
            continue
        if not is_managed_module_name(path.name):
            continue
        if not has_csharp_sources(path):
            continue
        modules.append(path)
    return modules


def build_project_spec(module_dir: Path) -> ProjectSpec:
    module_name = module_dir.name
    override = MODULE_OVERRIDES.get(module_name, {})
    kind = template_kind(module_name, override)

    project_file = override.get("project_file", f"{module_name}.csproj")
    assembly_name = override.get("assembly_name", module_name)
    intermediate_name = override.get("intermediate_name", assembly_name)
    target_framework = str(override.get("target_framework", "net10.0"))

    properties: dict[str, str | bool] = {}

    if kind in ("editor_exe", "editor_library"):
        for key, value in EDITOR_DEFAULTS.items():
            properties[_to_msbuild_key(key)] = value
    else:
        for key, value in RUNTIME_LIBRARY_DEFAULTS.items():
            properties[_to_msbuild_key(key)] = value

    for key, value in override.get("extra_properties", {}).items():
        properties[key] = value

    if kind == "editor_exe":
        properties["OutputType"] = "Exe"

    if kind == "runtime" and module_name.startswith("Neverness.Runtime."):
        suffix = module_name.removeprefix("Neverness.Runtime.")
        properties.setdefault("RootNamespace", f"Neverness.Managed.{suffix}")

    properties["AssemblyName"] = assembly_name
    properties["TargetFramework"] = target_framework
    properties["BaseOutputPath"] = "$(SolutionDir)Build\\bin\\"
    properties["OutputPath"] = "$(BaseOutputPath)$(Configuration)\\"
    properties["IntermediateOutputPath"] = (
        f"$(SolutionDir)Build\\Intermediate\\{intermediate_name}\\$(Configuration)\\"
    )

    return ProjectSpec(
        module_dir=module_dir,
        module_name=module_name,
        project_file=project_file,
        assembly_name=assembly_name,
        intermediate_name=intermediate_name,
        template=kind,
        target_framework=target_framework,
        properties=properties,
        package_references=list(override.get("package_references", [])),
        project_references=list(override.get("project_references", [])),
    )


def _to_msbuild_key(snake: str) -> str:
    parts = snake.split("_")
    return "".join(p[:1].upper() + p[1:] for p in parts)


def _bool_text(value: str | bool) -> str:
    if isinstance(value, bool):
        return "true" if value else "false"
    return str(value)


def render_editor_csproj(spec: ProjectSpec, *, executable: bool) -> str:
    """Match NervernessEditor.csproj layout (tabs, blank lines, comments)."""
    im = spec.intermediate_name
    lines = [
        '<Project Sdk="Microsoft.NET.Sdk">',
        "\t<PropertyGroup>",
        f"\t\t<AssemblyName>{spec.assembly_name}</AssemblyName>",
    ]
    if executable:
        lines.append("\t\t<OutputType>Exe</OutputType>")
    lines.extend(
        [
            f"\t\t<TargetFramework>{spec.target_framework}</TargetFramework>",
            "",
            "\t\t<ImplicitUsings>enable</ImplicitUsings>",
            "\t\t<Nullable>enable</Nullable>",
            "",
            "\t\t<EnableNativeCodeDebugging>true</EnableNativeCodeDebugging>",
            "\t\t<DebugType>portable</DebugType>",
            "",
            "\t\t<BaseOutputPath>$(SolutionDir)Build\\bin\\</BaseOutputPath>",
            "\t\t<OutputPath>$(BaseOutputPath)$(Configuration)\\</OutputPath>",
            "",
            "\t\t<!-- 强制独立 Intermediate -->",
            "\t\t<IntermediateOutputPath>",
            f"\t\t\t$(SolutionDir)Build\\Intermediate\\{im}\\$(Configuration)\\",
            "\t\t</IntermediateOutputPath>",
            "",
            "\t\t<GenerateAssemblyInfo>false</GenerateAssemblyInfo>",
            "",
            "\t\t<!-- 关键 -->",
            "\t\t<GenerateTargetFrameworkAttribute>false</GenerateTargetFrameworkAttribute>",
            "",
            "\t\t<AppendTargetFrameworkToOutputPath>false</AppendTargetFrameworkToOutputPath>",
            "\t\t<AppendRuntimeIdentifierToOutputPath>false</AppendRuntimeIdentifierToOutputPath>",
        ]
    )

    extras = [
        (key, spec.properties[key])
        for key in sorted(spec.properties)
        if key not in EDITOR_STANDARD_KEYS
    ]
    if extras:
        lines.append("")
        for key, value in extras:
            lines.append(f"\t\t<{key}>{_bool_text(value)}</{key}>")

    lines.append("\t</PropertyGroup>")
    lines.extend(_render_item_groups(spec, indent="\t"))
    lines.append("</Project>")
    lines.append("")
    return "\n".join(lines)


def render_runtime_csproj(spec: ProjectSpec) -> str:
    lines = [
        "<Project Sdk=\"Microsoft.NET.Sdk\">",
        "  <PropertyGroup>",
    ]
    priority = [
        "AssemblyName",
        "TargetFramework",
        "Nullable",
        "ImplicitUsings",
        "IsPackable",
        "AllowUnsafeBlocks",
        "RollForward",
        "RootNamespace",
        "BaseOutputPath",
        "OutputPath",
        "IntermediateOutputPath",
        "AppendTargetFrameworkToOutputPath",
        "AppendRuntimeIdentifierToOutputPath",
    ]
    written: set[str] = set()
    for key in priority:
        if key in spec.properties:
            lines.append(f"    <{key}>{_bool_text(spec.properties[key])}</{key}>")
            written.add(key)
    for key in sorted(spec.properties):
        if key not in written:
            lines.append(f"    <{key}>{_bool_text(spec.properties[key])}</{key}>")

    lines.append("  </PropertyGroup>")
    lines.extend(_render_item_groups(spec, indent="  "))
    lines.append("</Project>")
    lines.append("")
    return "\n".join(lines)


def _render_item_groups(spec: ProjectSpec, *, indent: str) -> list[str]:
    lines: list[str] = []
    tab = indent + "\t" if indent == "\t" else indent + "  "

    if spec.package_references:
        lines.append(f"{indent}<ItemGroup>")
        for name, version in spec.package_references:
            if name == "xunit.runner.visualstudio":
                lines.extend(
                    [
                        f'{tab}<PackageReference Include="{name}" Version="{version}">',
                        f"{tab}\t<PrivateAssets>all</PrivateAssets>",
                        f"{tab}\t<IncludeAssets>runtime; build; native; contentfiles; analyzers; buildtransitive</IncludeAssets>",
                        f"{tab}</PackageReference>",
                    ]
                )
            else:
                lines.append(
                    f'{tab}<PackageReference Include="{name}" Version="{version}" />'
                )
        lines.append(f"{indent}</ItemGroup>")

    if spec.project_references:
        lines.append(f"{indent}<ItemGroup>")
        for ref in spec.project_references:
            lines.append(f'{tab}<ProjectReference Include="{ref}" />')
        lines.append(f"{indent}</ItemGroup>")

    return lines


def render_csproj(spec: ProjectSpec) -> str:
    if spec.template == "editor_exe":
        return render_editor_csproj(spec, executable=True)
    if spec.template == "editor_library":
        return render_editor_csproj(spec, executable=False)
    return render_runtime_csproj(spec)


def generate_projects(
    managed_root: Path,
    *,
    force: bool,
    dry_run: bool,
    only_modules: set[str] | None,
) -> int:
    module_dirs = discover_module_dirs(managed_root)
    if only_modules:
        module_dirs = [d for d in module_dirs if d.name in only_modules]
        missing = only_modules - {d.name for d in module_dirs}
        if missing:
            print(f"Unknown module(s): {', '.join(sorted(missing))}", file=sys.stderr)
            return 1

    if not module_dirs:
        print(f"No modules found under {managed_root}", file=sys.stderr)
        return 1

    created = updated = skipped = 0
    for module_dir in module_dirs:
        spec = build_project_spec(module_dir)
        output_path = module_dir / spec.project_file

        exists = output_path.exists()
        if exists and not force and not dry_run:
            skipped += 1
            continue

        content = render_csproj(spec)
        action = "update" if exists else "create"

        if dry_run:
            if exists and not force:
                print(f"# exists (would skip write; use --force): {output_path}")
            else:
                print(f"# would {action}: {output_path}")
            print(content)
            print()
            continue

        output_path.write_text(content, encoding="utf-8", newline="\n")
        if action == "create":
            created += 1
            print(f"Created {output_path}")
        else:
            updated += 1
            print(f"Updated {output_path}")

    if dry_run:
        print(f"# dry-run: {len(module_dirs)} module(s)", file=sys.stderr)
    else:
        print(
            f"Done: {created} created, {updated} updated, {skipped} skipped "
            f"(use --force to overwrite existing)"
        )
    return 0


def parse_args() -> argparse.Namespace:
    default_repo = repo_root_from_script()
    parser = argparse.ArgumentParser(
        description="Generate .csproj files for Engine/Source/Managed modules.",
    )
    parser.add_argument(
        "--repo-root",
        type=Path,
        default=default_repo,
        help=f"Repository root (default: {default_repo})",
    )
    parser.add_argument(
        "--module",
        action="append",
        dest="modules",
        metavar="NAME",
        help="Only generate for module directory name(s), e.g. Neverness.Runtime.Core",
    )
    parser.add_argument(
        "--force",
        action="store_true",
        help="Overwrite existing .csproj files",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Print generated XML instead of writing files",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    repo_root = args.repo_root.resolve()
    managed_root = repo_root / MANAGED_REL

    if not managed_root.is_dir():
        print(f"Managed root not found: {managed_root}", file=sys.stderr)
        return 1

    only_modules = set(args.modules) if args.modules else None
    return generate_projects(
        managed_root,
        force=args.force,
        dry_run=args.dry_run,
        only_modules=only_modules,
    )


if __name__ == "__main__":
    raise SystemExit(main())
