# Put here checks that exit with 0 for the cases when you don't want the CI
# Release Publisher to run.

# Don't add exiting on "$TRAVIS_TEST_RESULT" != "0" in here as some of the
# scripts have to run even if the job has failed. The scripts that should not
# run on a job failing already include such a check and exit on their own.

# We don't want to run CI Release Publisher in PRs.
# Travis-CI secure environment variables are not available in PRs, so running
# CI running CI Release Publisher in PRs would just error due to
# [CIRP_]GITHUB_ACCESS_TOKEN not being set.

if [ ! -z "$TRAVIS_EVENT_TYPE" ] && [ "$TRAVIS_EVENT_TYPE" != "cron" ]; then
  echo "Skipping publishing in a non-cron build"
  exit 0
fi

if [ ! -z "$TRAVIS_PULL_REQUEST" ] && [ "$TRAVIS_PULL_REQUEST" != "false" ]; then
  echo "Skipping publishing in a Pull Request"
  exit 0
fi
