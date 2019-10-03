#!/usr/bin/env python3
#
# Copyright (c) 2019, Linaro Limited
#
# SPDX-License-Identifier: BSD-2-Clause

import argparse


def get_args():
    parser = argparse.ArgumentParser(description='Helper script that updates '
                                     'the CHANGELOG.md file.\n'
                                     'Usage example:\n'
                                     '  ./update_changelog.py '
                                     ' --changelog-file CHANGELOG.md'
                                     ' --release-version 3.7.0'
                                     ' --previous-release-version 3.6.0'
                                     ' --release-date 2019-10-11'
                                     ' --previous-release-date 2019-07-05')

    parser.add_argument('--changelog-file', action='store', required=True,
                        help='Changelog file to be updated.')

    parser.add_argument('--release-date', action='store', required=True,
                        help='The release date (yyyy-mm-dd).')

    parser.add_argument('--previous-release-date', action='store',
                        required=True,
                        help='The previous release date (yyyy-mm-dd).')

    parser.add_argument('--release-version', action='store', required=True,
                        help='Release version (MAJOR.MINOR.PATCH).')

    parser.add_argument('--previous-release-version', action='store',
                        required=True,
                        help='Previous release version (MAJOR.MINOR.PATCH).')

    return parser.parse_args()


def prepend_write(filename, text):
    with open(filename, 'r+') as f:
        current_content = f.read()
        f.seek(0, 0)
        f.write(text + '\n' + current_content)
        f.flush()


def main():
    global args

    args = get_args()

    # Shorten name
    clf = args.changelog_file
    rd = args.release_date
    prd = args.previous_release_date
    rv = args.release_version
    prv = args.previous_release_version

    # In some cases we need underscore in string
    rvu = rv.replace('.', '_')

    text = "# OP-TEE - version {} ({})\n".format(rv, rd)
    text += "\n"
    text += "- Link to the GitHub [release page][github_release_{}] " \
            "(optee_os)\n".format(rvu)
    text += "- Links to the commits and pull requests merged into this " \
            "release for:\n"

    gits = ["OP-TEE/optee_os", "OP-TEE/optee_client", "OP-TEE/optee_test",
            "OP-TEE/build", "linaro-swg/optee_examples"]

    for g in gits:
        gu = g.replace('/', '_')
        gu = gu.replace('-', '_')
        text += "  - {}: [commits][{}_commits_{}] and [pull requests]" \
                "[{}_pr_{}]\n".format(g, gu, rvu, gu, rvu)

    text += "\n"
    text += "[github_release_{}]: https://github.com/OP-TEE/optee_os/" \
            "releases/tag/{}\n".format(rvu, rv)

    for g in gits:
        gu = g.replace('/', '_')
        gu = gu.replace('-', '_')
        text += "[{}_commits_{}]: https://github.com/{}/compare/" \
                "{}...{}\n".format(gu, rvu, g, prv, rv)
        text += "[{}_pr_{}]: https://github.com/{}/pulls?q=is%3Apr+is%3A" \
                "merged+base%3Amaster+merged%3A{}..{}\n".format(
                        gu, rvu, g, prd, rd)

    prepend_write(args.changelog_file, text)


if __name__ == "__main__":
    main()
