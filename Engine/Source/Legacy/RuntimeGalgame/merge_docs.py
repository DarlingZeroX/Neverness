import pathlib

def merge_docs():
    """
    合并 RuntimeGalgame 下各子模块 Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md，
    并追加根目录 GALGAME_MODULE_ARCHITECTURE_AND_PROGRESS.md。

    用法：在 Engine/Source/RuntimeGalgame 目录执行
        python merge_docs.py

    输出：MERGED_ARCHITECTURE_AND_PROGRESS.md（覆盖）
    """
    current_dir = pathlib.Path.cwd()
    output_file = current_dir / "MERGED_ARCHITECTURE_AND_PROGRESS.md"
    root_append_file = current_dir / "GALGAME_MODULE_ARCHITECTURE_AND_PROGRESS.md"

    sub_docs = sorted(current_dir.glob("*/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md"))

    with open(output_file, "w", encoding="utf-8") as outfile:
        outfile.write(
            "# MERGED — RuntimeGalgame 模块文档合集\n\n"
            "本文件由 `merge_docs.py` 自动生成，**请勿手改**；编辑请修改各子目录 "
            "`Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md` 或根目录 `GALGAME_MODULE_ARCHITECTURE_AND_PROGRESS.md` 后重新运行脚本。\n\n"
            f"共收录 **{len(sub_docs)}** 个子模块文档 + 根总览。\n\n"
        )

        for doc_path in sub_docs:
            module_name = doc_path.parent.parent.name
            print(f"正在处理模块: {module_name}")

            outfile.write(f"\n---\n## Module: {module_name}\n\n")
            with open(doc_path, "r", encoding="utf-8") as infile:
                outfile.write(infile.read())
            outfile.write("\n\n")

        if root_append_file.exists():
            print(f"正在追加根目录文件: {root_append_file.name}")
            outfile.write(f"\n---\n## FinalOverview: {root_append_file.name}\n\n")
            with open(root_append_file, "r", encoding="utf-8") as infile:
                outfile.write(infile.read())
        else:
            print(f"警告: 未找到根目录文件 {root_append_file.name}")

    print(f"\n成功！合并后的文件已输出至: {output_file}")

if __name__ == "__main__":
    merge_docs()
