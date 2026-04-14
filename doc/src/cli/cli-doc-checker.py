#!/usr/bin/env python

import argparse
from enum import Enum
import re
import subprocess
from typing import Optional


class DocRegion(Enum):
    OTHER = 1
    USAGE = 2
    POSARGS = 3
    OPTARGS = 4


class MessageType(Enum):
    ERROR = "ERROR"
    WARNING = "WARNING"
    NOTE = "NOTE"


class ArgType(Enum):
    POSITIONAL = "positional"
    OPTIONAL = "optional"


def message(message: str, message_type: MessageType) -> None:
    print(f"{message_type.value}: {message}")


# commands to ignore, for example because they're deprecated
# ignored commands will not raise diagnostics if their arguments are not
# documented, but will still raise errors if they are completely missing
# from documentation.
ignored_commands = [
    "kicad-cli pcb export hpgl",
    "kicad-cli sch export hpgl",
]


class Command:
    def __init__(self, command: list[str]) -> None:
        self.command = command
        self.subcommands: dict[str, "Command"] = {}

        self.short_command = " ".join(command)
        if "kicad-cli" in self.short_command:
            self.short_command = self.short_command[
                self.short_command.find("kicad-cli") :
            ]
        self.last_subcommand = self.short_command.split()[-1]

        print("running " + " ".join(command))
        p = subprocess.run(command + ["--help"], capture_output=True, text=True)
        self.output = p.stdout

        usage = self.get_usage_from_output()
        argstring = re.search(f"Usage: .*{self.last_subcommand} (.*)", usage)
        self.args: list[str] = []
        if argstring:
            for a in argstring.group(1).split():
                if self.args and "[" not in a and "]" in a and "[" in self.args[-1]:
                    # incomplete second half of the arg, like 'VAR]'
                    self.args[-1] = f"{self.args[-1]} {a}"
                else:
                    self.args.append(a)

        self.posargs = self.get_pos_args_from_output()
        self.optargs = self.get_opt_args_from_output()

        self.documented = False
        self.documented_posargs: list[str] = []
        self.documented_optargs: list[str] = []

        subcommand_names = self.get_subcommand_names_from_output()
        for s in subcommand_names:
            self.subcommands[s] = Command(command + s.split())

    def get_subcommands(self, only_leaves=False) -> list[str]:
        subcommands_to_document = []
        if not only_leaves or not self.subcommands:
            # only include self if we're a leaf or if we're tracking branches too
            subcommands_to_document.append(self.short_command)
        for s in self.subcommands:
            subcommands_to_document += self.subcommands[s].get_subcommands(only_leaves)
        return subcommands_to_document

    def get_subcommand_by_name(self, name: str) -> Optional["Command"]:
        if name == self.short_command:
            return self
        for s in self.subcommands:
            result = self.subcommands[s].get_subcommand_by_name(name)
            if result:
                return result
        return None

    def build_usage(self) -> str:
        return f"Usage: `{self.short_command} {' '.join(self.args)}`\n"

    def get_subcommand_names_from_output(self) -> list[str]:
        lines = self.output.rstrip().splitlines()
        if "Subcommands:" in lines:
            subcommand_index = lines.index("Subcommands:") + 1
            return [line.split()[0] for line in lines[subcommand_index:]]
        return []

    def get_usage_from_output(self) -> str:
        usage = re.search("^Usage: .*$", self.output, re.MULTILINE)
        if usage:
            return usage.group(0)
        return ""

    def get_pos_args_from_output(self) -> list[str]:
        region = DocRegion.OTHER
        posargs = []
        for line in self.output.splitlines():
            if "Positional arguments" in line:
                region = DocRegion.POSARGS
                continue
            if "Optional arguments" in line:
                # stop looking for positional args now
                region = DocRegion.OPTARGS
                continue
            if region == DocRegion.POSARGS:
                args = re.search(r"^\s*([A-Z0-9_]+).*", line)
                if args:
                    posargs.append(args.group(1).replace(" ", ""))
        return posargs

    def get_opt_args_from_output(self) -> list[str]:
        region = DocRegion.OTHER
        optargs = []
        for line in self.output.splitlines():
            if "Optional arguments" in line:
                region = DocRegion.OPTARGS
                continue
            if region == DocRegion.OPTARGS:
                args = re.search(r"^\s*(-[-\w]+(?:, -[-\w]+)?).*", line)
                if args:
                    optargs.append(args.group(1).replace(" ", ""))
        return optargs

    def check_documented_args_match_real_args(self) -> None:
        def _check_args(argtype: ArgType) -> None:
            if argtype == ArgType.POSITIONAL:
                cmd_args = list(self.posargs)
                doc_args = list(self.documented_posargs)
            else:
                cmd_args = list(self.optargs)
                doc_args = list(self.documented_optargs)
            if cmd_args and not doc_args:
                message(
                    f"no {argtype.value} arguments found in documentation for `{self.short_command}`. "
                    f"Arguments undocumented or missing '{argtype.value.capitalize()} arguments:' section header?",
                    MessageType.ERROR,
                )
                return  # don't bother with rest of the diagnostics, they'll just be misleading
            for arg in doc_args:
                if arg not in cmd_args:
                    doc_args.remove(arg)
                    message(
                        f"{argtype.value} argument `{arg}` is documented but not real for `{self.short_command}`",
                        MessageType.WARNING,
                    )
            # check for args that aren't documented
            for arg in cmd_args:
                if arg not in doc_args:
                    message(
                        f"undocumented {argtype.value} argument `{arg}` for `{self.short_command}`",
                        MessageType.WARNING,
                    )
                elif doc_args and arg != doc_args[0]:
                    message(
                        f"{argtype.value} argument `{arg}` is out-of-order in documentation for `{self.short_command}`",
                        MessageType.WARNING,
                    )
                    doc_args.remove(arg)
                else:
                    doc_args.pop(0)

        if not self.documented:
            message(f"no documentation found for `{self.short_command}`", MessageType.ERROR)
            return  # don't bother with rest of the diagnostics, they'll just be misleading
        if self.short_command in ignored_commands:
            return
        _check_args(ArgType.POSITIONAL)
        _check_args(ArgType.OPTIONAL)


def process_docs(leaf_subcommands: list[str]) -> None:
    with open("cli.adoc", "r") as infile, open("cli.adoc.out", "w") as outfile:
        region = DocRegion.OTHER
        subcommand = None
        for n, line in enumerate(infile, start=1):
            heading = re.search(r"^[=]+\s+(.*)", line)
            if heading:
                # reset state
                subcommand = None
                region = DocRegion.OTHER

            usage = re.search(r"^Usage:.*(kicad-cli(?: [a-z0-9_\-]+)*).*", line)
            if usage:
                region = DocRegion.USAGE
                old_usage = usage.group(0)
                subcommand_name = usage.group(1)
                subcommand = root.get_subcommand_by_name(subcommand_name)
                if subcommand:
                    subcommand.documented = True
                    new_usage = subcommand.build_usage()
                    outfile.write(new_usage)
                    if old_usage != new_usage.strip():  # new usage has a trailing newline
                        message(
                            f"updating usage for `{subcommand_name}` at cli.adoc:{n}",
                            MessageType.NOTE,
                        )
                else:
                    message(
                        f"unknown command `{subcommand_name}` at cli.adoc:{n}",
                        MessageType.ERROR,
                    )
                continue
            else:
                outfile.write(line)

            posargs = re.search(r"^Positional arguments:.*", line)
            if posargs:
                # skip processing whole posargs section if we didn't figure
                # out what subcommand we're talking about
                if subcommand:
                    region = DocRegion.POSARGS
                else:
                    message(
                        f"unable to determine subcommand before positional arguments section at cli.adoc:{n}",
                        MessageType.ERROR,
                    )
            if region == DocRegion.POSARGS:
                posarg = re.search(r"\s*\|\s*`([A-Z0-9_]+)`.*", line)
                if subcommand and posarg:
                    subcommand.documented_posargs.append(posarg.group(1))

            optargs = re.search(r"^Optional arguments:.*", line)
            if optargs:
                # skip processing whole optargs section if we didn't figure
                # out what subcommand we're talking about
                if subcommand:
                    region = DocRegion.OPTARGS
                else:
                    message(
                        f"unable to determine subcommand before optional arguments section at cli.adoc:{n}",
                        MessageType.ERROR,
                    )
            if region == DocRegion.OPTARGS:
                optarg = re.search(
                    r"\s*\|\s*(`-[-\w <>=]+`(?:, `-[-\w <>=]+`)?).*", line
                )
                if subcommand and optarg:
                    a = re.sub(r"[` =]", "", optarg.group(1))
                    a = re.sub(r"\s*<[\w-]*>", "", a)
                    subcommand.documented_optargs.append(a)

        for name in leaf_subcommands:
            subcommand = root.get_subcommand_by_name(name)
            if subcommand:
                subcommand.check_documented_args_match_real_args()


if __name__ == "__main__":
    usage = (
        "Checks accuracy of cli.adoc against kicad-cli output. "
        "Usage lines are automatically updated and written to cli.adoc.out. "
        "Other discrepancies are reported to stdout but not automatically fixed."
     )
    parser = argparse.ArgumentParser(prog="cli-doc-checker", description=usage)
    parser.add_argument("kicad_cli_path", help="full path to kicad-cli binary")
    args = parser.parse_args()

    root = Command([args.kicad_cli_path])
    subcommands = root.get_subcommands()
    leaf_subcommands = root.get_subcommands(only_leaves=True)
    process_docs(leaf_subcommands)
