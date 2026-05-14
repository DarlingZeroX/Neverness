import pathlib

def merge_docs():
    """
    合并 RuntimeGalgame 下各子模块 Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md，
    并追加根目录 MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md。

    用法：在 Engine/Source/Managed 目錄執行
        python merge_docs.py

    输出：MERGED_ARCHITECTURE_AND_PROGRESS.md（覆盖）
    """
    current_dir = pathlib.Path.cwd()
    output_file = current_dir / "MERGED_ARCHITECTURE_AND_PROGRESS.md"
    root_append_file = current_dir / "MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md"

    sub_docs = sorted(current_dir.glob("*/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md"))
    runtime_engine_abi = (
        current_dir.parent / "Runtime" / "VGNativeEngineAPI" / "Docs" / "MODULE_ARCHITECTURE_AND_PROGRESS.md"
    )
    if runtime_engine_abi.is_file():
        sub_docs = sorted(sub_docs + [runtime_engine_abi], key=lambda p: str(p).replace("\\", "/"))

    with open(output_file, "w", encoding="utf-8") as outfile:
        outfile.write(
            "# MERGED — Managed 子樹模組文檔合集\n\n"
            "本文件由 `merge_docs.py` 自動生成，**請勿手改**；請編輯各子目錄 "
            "`Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md`、根目錄 `MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md`，"
            "以及（可選）`Engine/Source/Runtime/VGNativeEngineAPI/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md` 後重新執行本腳本。\n\n"
            f"共收錄 **{len(sub_docs)}** 個子模組文檔 + 根總覽。\n\n"
        )

        for doc_path in sub_docs:
            module_name = doc_path.parent.parent.name
            print(f"正在處理模組: {module_name}")

            outfile.write(f"\n---\n## Module: {module_name}\n\n")
            with open(doc_path, "r", encoding="utf-8") as infile:
                outfile.write(infile.read())
            outfile.write("\n\n")

        if root_append_file.exists():
            print(f"正在追加根目錄檔案: {root_append_file.name}")
            outfile.write(f"\n---\n## FinalOverview: {root_append_file.name}\n\n")
            with open(root_append_file, "r", encoding="utf-8") as infile:
                outfile.write(infile.read())
        else:
            print(f"警告: 未找到根目錄檔案 {root_append_file.name}")

    print(f"\n成功！合併後的檔案已輸出至: {output_file}")

if __name__ == "__main__":
    merge_docs()
