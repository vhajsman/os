#!/usr/bin/env python3

import re

# =======================================
# HELPER FUNCTIONS - PRINT
# =======================================

print_lvl = 0

def print_lvl_up():
    global print_lvl
    print_lvl = print_lvl + 1

def print_lvl_down():
    global print_lvl
    if(print_lvl != 0): print_lvl = print_lvl - 1

def print_sub(msg, end="\n"):
    if(print_lvl > 0):
        print(f"{"-" * print_lvl}> ", end="")
    
    print(msg, end=end)

# =======================================
# 
# =======================================

f_input = "config/kernel_config.h"
f_output = ".configure"

config_vars = []


def parseConfig():
    print_sub("Parse kernel_config.h")

    with open(f_input) as fin:
        lines = fin.readlines()
    

    print_lvl_up()

    i = 0
    while i < len(lines):
        line = lines[i].strip()

        # Comment start
        if line.startswith("/*"):
            comment_lines = []
            i += 1

            while i < len(lines) and "*/" not in lines[i]:
                comment_lines.append(lines[i].strip(" *\n\r"))
                i += 1
            
            if i < len(lines):
                comment_lines.append(lines[i].strip(" */\n\r"))
                i += 1
            
            # Parse description, type
            _desc = ""
            _type = ""
            _default = ""

            for c_line in comment_lines:
                if c_line.lower().startswith("type:"):          _type       = c_line.split(":",1)[1].strip().lower()
                elif c_line.lower().startswith("default:"):     _default    = c_line.split(":",1)[1].strip()
                elif c_line.strip() != "" and _desc == "":      _desc       = c_line.strip()
            
            if i < len(lines):
                define_line = lines[i].strip()

                m = re.match(r"#define\s+(CONFIG_\w+)\s+(.+)", define_line)
                if m:
                    macro, value = m.groups()
                    config_vars.append({
                        "macro": macro,
                        "desc": _desc,
                        "type": _type,
                        "default": _default,
                        "current": value.strip(),
                    })

                    print_sub(f"found macro: {macro} --- [type={_type},default=${_default}]")
                
                i += 1
            
        else:
            i += 1
        
        i += 1
    
    print_lvl_down()

def genInteractive():
    print_sub("Generate interactive configure script")
    print_lvl_up()

    i=0

    with open(f_output, "w") as out:
        out.write("#!/usr/bin/env python3\n")
        out.write("import re\nfrom dialog import Dialog\n\n")
        out.write(f"CONFIG_FILE = \"{f_input}\"\n\n")

        out.write("# ============================================\n")
        out.write("# pre-generated options list. generated using make mkconfig\n")
        out.write("# ============================================\n\n")

        out.write("options = [\n")
        for c in config_vars:
            print_sub(f"paste entry for: {c['macro']} ({i})")

            out.write(f"   # --- OPTION {i}: {c["macro"]} ---\n")
            out.write("    {\n")
            out.write(f"        'macro': '{c['macro']}',\n")
            out.write(f"        'desc': '''{c['desc']}''',\n")
            out.write(f"        'type': '{c['type']}',\n")
            out.write(f"        'default': '{c['default']}',\n")
            out.write(f"        'current': '{c['current']}'\n")
            out.write("    },\n\n")
        out.write("]\n\n")

        print_sub(f"Generated entries for total of {i + 1} macros, skipped 0")

        out.write("""
def update_config_file(options):
    with open(CONFIG_FILE, "r") as f:
        lines = f.readlines()

    for i, line in enumerate(lines):
        m = re.match(r"#define\\s+(CONFIG_\\w+)\\s+(.+)", line)
        if m:
            macro = m.group(1)
            for opt in options:
                if opt["macro"] == macro and "new_val" in opt:
                    lines[i] = f"#define {macro} {opt['new_val']}\\n"

    with open(CONFIG_FILE, "w") as f:
        f.writelines(lines)


def main():
    d = Dialog(dialog='dialog')
    d.set_background_title("Kernel Configuration")

    for opt in options:
        macro = opt['macro']
        desc = opt['desc']
        typ = opt['type']
        default = opt['default']
        current = opt['current'].strip('"')

        if typ == "bool":
            code = d.yesno(f"{desc} ({macro})", yes_label="Yes", no_label="No", default_button=0 if current == "1" else 1)
            opt["new_val"] = "1" if code == d.OK else "0"

        elif typ == "string":
            code, val = d.inputbox(f"{desc} ({macro})", init=current or default.strip('"'))
            if code == d.OK:
                opt["new_val"] = f'"{val}"'
            else:
                opt["new_val"] = f'"{current}"'

        elif typ == "int":
            code, val = d.inputbox(f"{desc} ({macro})", init=current or default)
            if code == d.OK and val.isdigit():
                opt["new_val"] = val
            else:
                opt["new_val"] = current

    update_config_file(options)
    d.msgbox("✓ Configuration saved to " + CONFIG_FILE)


if __name__ == "__main__":
    main()
""")

if __name__ == "__main__":
    parseConfig()
    genInteractive()

    print("\n\n")
    print("-------------------------------")
    print("Generation complete!")
    print("")
    print("The only thing this has generated is an interactive")
    print("configuration script. To run this script, run:")
    print("    ./configure")
    print("")
    print("To run the generation again, run:")
    print("    make mkconfig")
    print("")
    print("Have a great day! :3")