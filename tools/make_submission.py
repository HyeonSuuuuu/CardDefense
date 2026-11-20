"""제출용 폴더 만들기 (과제 안내 PDF 6쪽: 소스·셰이더·리소스·솔루션, Release 실행 파일, 라이브러리).

먼저 Visual Studio에서 x64 / Release로 빌드한 뒤 실행한다.
    python tools/make_submission.py

결과: 제출/CardDefense-제출/
    실행파일/   Release exe + 셰이더 + Data + assets + DLL (GLEW·GLFW + VC++ 런타임)
                → 다른 PC에서 VC++ 재배포 패키지 없이도 CardDefense.exe를 더블클릭하면 실행된다.
    프로젝트/   솔루션·소스·셰이더·Data, 게임이 쓰는 assets, external(GLEW·GLFW·GLM)
                → CardDefense/CardDefense.slnx를 열어 바로 빌드할 수 있다.
보고서·발표 자료는 직접 폴더에 넣고 압축한다.
"""
import glob
import os
import shutil
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, "제출", "CardDefense-제출")
RELEASE = os.path.join(ROOT, "CardDefense", "x64", "Release")

# VC++ 런타임: exe가 쓰는 MSVCP140·VCRUNTIME140·VCRUNTIME140_1 (dumpbin /dependents로 확인)
CRT_DLLS = ["msvcp140.dll", "vcruntime140.dll", "vcruntime140_1.dll"]

# 게임이 실행 중에 읽는 assets (vcxproj의 CopyGameFiles와 같은 범위)
GAME_ASSETS = ["assets/obj", "assets/textures", "assets/cards/card_*.png", "assets/ui/*.png"]

# 프로젝트 폴더에서 뺄 것 (빌드 결과물, 개인 설정)
IGNORE = shutil.ignore_patterns("x64", "Win32", ".vs", "*.user", "*.pdb", "*.ilk", "*.obj", "*.tlog", "*.log")


def find_crt_dir():
    pattern = r"C:\Program Files\Microsoft Visual Studio\*\*\VC\Redist\MSVC\*\x64\Microsoft.VC*.CRT"
    # 빌드한 도구 집합보다 오래된 런타임이면 실행되지 않으므로 MSVC 버전 번호(14.51.xxxxx)가 가장 큰 것을 고른다.
    def version(path):
        name = os.path.basename(os.path.dirname(os.path.dirname(path)))
        return tuple(int(x) for x in name.split(".") if x.isdigit())
    found = sorted(glob.glob(pattern), key=version)
    return found[-1] if found else None


def remove_empty_dirs(root):
    for folder, dirs, files in os.walk(root, topdown=False):
        if not dirs and not files:
            os.rmdir(folder)


def copy_assets(dest_root):
    for pattern in GAME_ASSETS:
        for src in glob.glob(os.path.join(ROOT, pattern)):
            rel = os.path.relpath(src, ROOT)
            dst = os.path.join(dest_root, rel)
            os.makedirs(os.path.dirname(dst), exist_ok=True)
            if os.path.isdir(src):
                shutil.copytree(src, dst)
            else:
                shutil.copy2(src, dst)


def main():
    exe = os.path.join(RELEASE, "CardDefense.exe")
    if not os.path.exists(exe):
        sys.exit("Release 빌드가 없다: " + exe)
    crt = find_crt_dir()
    if not crt:
        sys.exit("VC++ 런타임 폴더(VC\\Redist\\MSVC\\...\\x64\\Microsoft.VC*.CRT)를 찾지 못했다")

    if os.path.exists(OUT):
        shutil.rmtree(OUT)

    # 1. 실행파일: Release 폴더(빌드 때 셰이더·Data·assets·DLL이 이미 복사됨)에서 pdb만 빼고 복사
    run_dir = os.path.join(OUT, "실행파일")
    shutil.copytree(RELEASE, run_dir, ignore=shutil.ignore_patterns("*.pdb", "*.iobj", "*.ipdb"))
    for name in CRT_DLLS:
        shutil.copy2(os.path.join(crt, name), run_dir)

    # 2. 프로젝트: 솔루션 폴더 + 게임 assets + external + README
    proj_dir = os.path.join(OUT, "프로젝트")
    shutil.copytree(os.path.join(ROOT, "CardDefense"), os.path.join(proj_dir, "CardDefense"), ignore=IGNORE)
    shutil.copytree(os.path.join(ROOT, "external"), os.path.join(proj_dir, "external"))
    copy_assets(proj_dir)
    shutil.copy2(os.path.join(ROOT, "README.md"), proj_dir)
    remove_empty_dirs(proj_dir)  # x64를 뺀 뒤 남는 빈 중간 폴더

    total = 0
    for folder, _, files in os.walk(OUT):
        total += sum(os.path.getsize(os.path.join(folder, f)) for f in files)
    print("완료:", OUT)
    print("크기: %.1f MB" % (total / 1024 / 1024))
    print("CRT:", crt)


if __name__ == "__main__":
    main()
