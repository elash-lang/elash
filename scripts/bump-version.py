#!/usr/bin/env python3
import sys
import os
import subprocess

def main():
    if len(sys.argv) != 2 or sys.argv[1] not in ['major', 'minor', 'patch']:
        print("Usage: bump-version.py {major|minor|patch}")
        sys.exit(1)

    bump_type = sys.argv[1]
    version_file = "VERSION"

    if not os.path.exists(version_file):
        with open(version_file, "w") as f:
            f.write("0.0.0\n")

    with open(version_file, "r") as f:
        current_version = f.read().strip()

    try:
        major, minor, patch = map(int, current_version.split('.'))
    except ValueError:
        print(f"Error: Invalid version format in {version_file}: '{current_version}'")
        sys.exit(1)

    if bump_type == 'major':
        major += 1
        minor = 0
        patch = 0
    elif bump_type == 'minor':
        minor += 1
        patch = 0
    elif bump_type == 'patch':
        patch += 1

    new_version = f"{major}.{minor}.{patch}"
    
    with open(version_file, "w") as f:
        f.write(new_version + "\n")

    print(f"Version bumped from {current_version} to {new_version}")

    if os.path.isdir(".git"):
        try:
            subprocess.run(["git", "add", version_file], check=True)
            subprocess.run(["git", "commit", "-m", f"release: bump version to v{new_version}"], check=True)
            subprocess.run(["git", "tag", "-a", f"v{new_version}", "-m", f"Release v{new_version}"], check=True)
            print(f"Git commit and tag v{new_version} created.")
        except subprocess.CalledProcessError as e:
            print(f"Warning: Git commands failed: {e}")

if __name__ == "__main__":
    main()
