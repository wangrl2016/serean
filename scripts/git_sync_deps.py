#!/usr/bin/python3

import os
import subprocess
import sys
import threading

deps = {
    'tutorials/juce/JUCE': 'https://github.com/juce-framework/JUCE.git@2f980209cc4091a4490bb1bafc5d530f16834e58',
}


def git_executable():
    """
    Find the git executable

    :return: A string suitable for passing to subprocess functions, or None.
    """
    search_list = ['git', 'git.bat']
    with open(os.devnull, 'w') as devnull:
        for git in search_list:
            try:
                subprocess.run(['git', '--version'], stdout=devnull)
            except (OSError,):
                continue
            return git
        return None


def is_sha1_sum(s):
    """
    SHA1 sums are 160 bits, encoded as lowercase hexadecimal.
    """
    return len(s) == 40 and all(c in '0123456789abcdef' for c in s)


def status(directory, commit_hash, change):
    def truncate_beginning(s, length):
        return s if len(s) <= length else '...' + s[-(length - 3):]

    def truncate_end(s, length):
        return s if len(s) <= length else s[:(length - 3)] + '...'

    dlen = 36
    directory = truncate_beginning(directory, dlen)
    commit_hash = truncate_end(commit_hash, 40)
    symbol = '>' if change else '@'
    sys.stdout.write('%-*s %s %s\n' % (dlen, directory, symbol, commit_hash))


def git_repository_sync_is_disabled(git, directory):
    try:
        disable = subprocess.check_output(
            [git, 'config', 'sync-deps.disable'], cwd=directory)
        return disable.lower().strip() in ['true', '1', 'yes', 'on']
    except subprocess.CalledProcessError:
        return False


def is_git_top_level(git, directory):
    """
    Return true if the directory is the top level of a Git repository.

    :param git: (string) the git executable
    :param directory: (string) the path into which the repository is expected to be checked out.
    """
    try:
        toplevel = subprocess.check_output(
            [git, 'rev-parse', '--show-toplevel'], cwd=directory).strip()
        return os.path.realpath(directory) == os.path.realpath(toplevel.decode())
    except subprocess.CalledProcessError:
        return False


def git_checkout_to_directory(git, repo, commit_hash, directory, verbose):
    """
    Checkout (and clone if needed) a Git repository.

    :param git: (string) the git executable
    :param repo: (string) the location of the repository, suitable for passing to `git clone`.
    :param commit_hash: (string) a commit, suitable for passing to `git checkout`
    :param directory: (string) the path into which the repository should be checked out.
    :param verbose: (boolean)

    Raises an exception if any calls to git fail.
    """
    if not os.path.isdir(directory):
        subprocess.check_call(
            [git, 'clone', '--no-checkout', repo, directory])

        subprocess.check_call([git, 'checkout', '--quiet', commit_hash],
                              cwd=directory)
        if verbose:
            status(directory, commit_hash, True)
        return

    if not is_git_top_level(git, directory):
        # if the directory exists, but isn't a git repo, you will modify
        # the parent repository, which isn't what you want.
        sys.stdout.write('%s\n  IS NOT TOP-LEVEL GIT DIRECTORY.\n' % directory)
        return

    # Check to see if this repo is disabled.  Quick return.
    if git_repository_sync_is_disabled(git, directory):
        sys.stdout.write('%s\n  SYNC IS DISABLED.\n' % directory)
        return

    with open(os.devnull, 'w') as devnull:
        # If this fails, we will fetch before trying again.  Don't spam user
        # with error information.
        if 0 == subprocess.call([git, 'checkout', '--quiet', commit_hash],
                                cwd=directory, stderr=devnull):
            # if this succeeds, skip slow `git fetch`.
            if verbose:
                status(directory, commit_hash, False)  # Success.
            return

    # If the repo has changed, always force use of the correct repo.
    # If origin already points to repo, this is a quick no-op.
    subprocess.check_call(
        [git, 'remote', 'set-url', 'origin', repo], cwd=directory)

    subprocess.check_call([git, 'fetch', '--quiet'], cwd=directory)

    subprocess.check_call([git, 'checkout', '--quiet', commit_hash], cwd=directory)

    if verbose:
        status(directory, commit_hash, True)  # Success.


def multi_thread(function, list_of_arg_lists):
    # for args in list_of_arg_lists:
    #   function(*args)
    # return
    threads = []
    for args in list_of_arg_lists:
        thread = threading.Thread(None, function, None, args)
        thread.start()
        threads.append(thread)

    for thread in threads:
        thread.join()


def git_sync_deps():
    """
    Grab dependencies, with optional platform support.

    :raise: git Exceptions
    """
    verbose = not bool(os.environ.get('GIT_SYNC_DEPS_QUIET', False))

    git = git_executable()
    assert git

    list_of_arg_lists = []

    for directory in sorted(deps):
        if '@' in deps[directory]:
            repo, commit_hash = deps[directory].split('@', 1)
        else:
            raise Exception('Please specify commit')

        if not is_sha1_sum(commit_hash):
            raise Exception('Poorly formed commit hash: %r' % commit_hash)

        list_of_arg_lists.append((
            git, repo, commit_hash, directory, verbose))

    multi_thread(git_checkout_to_directory, list_of_arg_lists)


if __name__ == '__main__':
    git_sync_deps()
