"""列出当前Runtime目录下的所有子目录，输出到Directory.md"""
import os
import pathlib

def main():
    # 获取脚本所在目录（即 Runtime 目录）
    runtime_dir = pathlib.Path(__file__).resolve().parent

    # 收集所有子目录名称（过滤掉以 . 开头的隐藏目录）
    dirs = sorted(
        d.name for d in runtime_dir.iterdir()
        if d.is_dir() and not d.name.startswith('.')
    )

    # 写入 Directory.md
    output_path = runtime_dir / "Directory.md"
    with open(output_path, "w", encoding="utf-8") as f:
        for name in dirs:
            f.write(name + "\n")

    print(f"已生成 {output_path}，共 {len(dirs)} 个子目录")

if __name__ == "__main__":
    main()
