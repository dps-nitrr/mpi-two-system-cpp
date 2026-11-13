# Uploading this project to GitHub

## Option 1: Command line

1. Create an **empty** repo on https://github.com/new (name e.g. `mpi-two-system-cpp`,
   do NOT tick "Add a README").
2. In this folder:

```bash
git init
git add .
git commit -m "MPI two-system data exchange (C++)"
git branch -M main
git remote add origin https://github.com/<your-username>/mpi-two-system-cpp.git
git push -u origin main
```
GitHub no longer accepts passwords for pushes: use a Personal Access Token
(Settings -> Developer settings -> Personal access tokens) as the password, or log in with `gh auth login`.

3. Add your screenshots later:

```bash
cp ~/Pictures/*.png screenshots/
git add screenshots && git commit -m "Add execution screenshots" && git push
```

## Option 2: No command line
Create the repo on GitHub, click **Add file -> Upload files**, drag in the contents of this folder
(keep the `src/` and `screenshots/` folders), and commit.

Submit the repository URL (make sure the repo is Public, or shared with your instructor).
