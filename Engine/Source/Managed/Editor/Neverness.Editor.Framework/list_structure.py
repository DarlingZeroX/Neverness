"""
通用模块目录结构输出脚本。
放在任意模块根目录下，直接运行即可。

用法：python list_structure.py

可通过修改下方 DIR_ALIAS 自定义子目录 → 命名空间映射。
"""

import os

ROOT = os.path.dirname(os.path.abspath(__file__))
MODULE_NAME = os.path.basename(ROOT)
EXCLUDE_DIRS = {'Build', 'obj', 'bin'}
SCRIPT_NAME = os.path.basename(__file__)

# ── 自定义配置 ──────────────────────────────────────────
# 子目录名 → 命名空间后缀映射（不填则自动用目录名）
# 例：'Private' → 'Internal' 表示 Private/ 目录对应 .Internal 命名空间
DIR_ALIAS: dict[str, str] = {}
# ────────────────────────────────────────────────────────


def to_namespace(rel_path: str) -> str:
    """相对路径转命名空间。根目录 = 模块名，子目录按路径拼接。"""
    if rel_path == '.':
        return MODULE_NAME
    parts = rel_path.replace('\\', '/').split('/')
    mapped = [DIR_ALIAS.get(p, p) for p in parts]
    return MODULE_NAME + '.' + '.'.join(mapped)


def count_cs(dirpath: str) -> int:
    """递归统计 .cs 文件数。"""
    total = 0
    for dirpath_, _, filenames in os.walk(dirpath):
        if any(ex in dirpath_.split(os.sep) for ex in EXCLUDE_DIRS):
            continue
        total += sum(1 for f in filenames if f.endswith('.cs'))
    return total


def get_entries(dirpath: str):
    """返回 (子目录列表, 文件列表)，排除 EXCLUDE_DIRS 和脚本自身。"""
    try:
        entries = sorted(os.listdir(dirpath))
    except PermissionError:
        return [], []
    dirs, files = [], []
    for name in entries:
        if name == SCRIPT_NAME:
            continue
        full = os.path.join(dirpath, name)
        if os.path.isdir(full):
            if name not in EXCLUDE_DIRS:
                dirs.append(name)
        else:
            files.append(name)
    return dirs, files


def print_tree(dirpath: str, prefix: str = ''):
    dirs, files = get_entries(dirpath)
    items = [(False, f) for f in files] + [(True, d) for d in dirs]

    for i, (is_dir, name) in enumerate(items):
        is_last = (i == len(items) - 1)
        connector = '\\---' if is_last else '+---'
        child_prefix = '    ' if is_last else '|   '

        if is_dir:
            full = os.path.join(dirpath, name)
            cs_count = count_cs(full)
            ns = to_namespace(os.path.relpath(full, ROOT))
            print(f'{prefix}{connector}{name}/ ({cs_count} .cs)  [{ns}]')
            print_tree(full, prefix + child_prefix)
        else:
            print(f'{prefix}{connector} {name}')


def main():
    cs_total = count_cs(ROOT)
    dir_total = sum(
        1 for dp, dn, _ in os.walk(ROOT)
        if not any(ex in dp.split(os.sep) for ex in EXCLUDE_DIRS) and dp != ROOT
    )

    print()
    print(f'{MODULE_NAME}/ ({dir_total} dirs, {cs_total} .cs files)')
    print('=' * 60)

    # 根目录文件
    _, root_files = get_entries(ROOT)
    for f in root_files:
        print(f'   {f}')

    # 目录树
    dirs, _ = get_entries(ROOT)
    for i, d in enumerate(dirs):
        full = os.path.join(ROOT, d)
        is_last = (i == len(dirs) - 1)
        connector = '\\---' if is_last else '+---'
        child_prefix = '    ' if is_last else '|   '
        cs_count = count_cs(full)
        ns = to_namespace(os.path.relpath(full, ROOT))
        print(f'{connector}{d}/ ({cs_count} .cs)  [{ns}]')
        print_tree(full, child_prefix)

    # 命名空间概览
    print()
    print('=' * 60)
    print('Namespaces:')
    seen = set()
    for dp, _, _ in os.walk(ROOT):
        if any(ex in dp.split(os.sep) for ex in EXCLUDE_DIRS):
            continue
        ns = to_namespace(os.path.relpath(dp, ROOT))
        if ns not in seen:
            seen.add(ns)
            rel = os.path.relpath(dp, ROOT).replace(os.sep, '/')
            print(f'  {rel}/ -> {ns}')
    print()


if __name__ == '__main__':
    main()
