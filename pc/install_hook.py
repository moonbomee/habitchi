import filecmp
import os
import shutil
import stat
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
HOOK_SOURCE = os.path.join(HERE, "hooks", "post-commit")


def hook_path(repo):
    result = subprocess.run(
        ["git", "rev-parse", "--git-path", "hooks/post-commit"],
        cwd=repo,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        universal_newlines=True,
    )
    if result.returncode != 0:
        return None
    return os.path.join(repo, result.stdout.strip())


def make_executable(path):
    mode = os.stat(path).st_mode
    os.chmod(path, mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)


def main():
    if len(sys.argv) != 2:
        print("사용법: python3 pc/install_hook.py <저장소 폴더>")
        return 1

    repo = os.path.abspath(os.path.expanduser(sys.argv[1]))
    if not os.path.isdir(repo):
        print("폴더가 없어요: " + repo)
        return 1

    target = hook_path(repo)
    if target is None:
        print("git 저장소가 아니에요: " + repo)
        return 1

    if os.path.exists(target):
        if filecmp.cmp(HOOK_SOURCE, target, shallow=False):
            make_executable(target)
            print("이미 해빗치 훅이 설치돼 있어요: " + target)
            return 0
        print("이 저장소에는 이미 다른 post-commit 훅이 있어요: " + target)
        print("덮어쓰지 않았어요. 기존 훅을 직접 확인한 뒤 옮기거나 합쳐 주세요.")
        return 1

    os.makedirs(os.path.dirname(target), exist_ok=True)
    shutil.copyfile(HOOK_SOURCE, target)
    make_executable(target)
    print("설치했어요: " + target)
    if os.path.exists(os.path.join(repo, ".habitchi-test")):
        print("테스트 명령(.habitchi-test)을 찾았어요. 커밋할 때마다 테스트를 돌려요.")
    else:
        print("테스트 명령(.habitchi-test)이 없어요. 커밋 메시지만 보내요.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
