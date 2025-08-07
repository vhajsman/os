#!/bin/python3

from kconfiglib import Kconfig

kconf = Kconfig("Kconfig")
kconf.load_config(".configure")

errors_occured = 0
properties_tot = 0

def write_error(_f, name, _type, error_msg="An unknown error has occured."):
    _f.write(f"// {"-" * (80 - 3)}\n")
    _f.write(f"    An error has occured.\n")
    _f.write(f"    PROPERTY NAME: {name}\n")
    _f.write(f"    PROPERTY TYPE: {_type}\n\n")
    _f.write("     DETAILS:\n")
    _f.write(f"    {error_msg}\n")
    _f.write(f"// {"-" * (80 - 3)}\n\n")

    global errors_occured
    errors_occured += 1


def main():
    with open("config/autoconf.h", "w") as f:
        global properties_tot

        f.write("""// CubeBox OS Kernel - Configuration file 
// ---------------------------------------------------------
// This file stores kernel build configuration information
// as macros.
// 
// CONFIGURATION FILE GENERATION
//     In order to generate the configuraiton file (this),
//     change working directory to the project root
//     directory and run:
//
//         make configure
//
//     This will execute Text-UI (ncurses) based configuraiton
//     tool.
// 
// PROPERTY TYPES
//     There are 3 different supported property types:
//       - BOOL   - (boolean) a simple true/false (yes/no) 
//                  value
//       - INT    - (number) a numerical value
//       - STRING - text value
//
//     Any unlisted property types will be considered
//     unknown, which causes an error.
//
// READING CONFIGURATION
//     To read configuration from kernel source code,
//     read the property as a constant macro.
// 
//     Macros are named this way:
//
//         _CONF_{PROPERTY_NAME}
// 
//     Use type cast / type conversion when needed.
// 
// This file is script-generated.
//

#ifndef _AUTOCONF_INCLUDED_H
#define _AUTOCONF_INCLUDED_H

#define AUTOCONF_LOAD(PROPERTY) \
    ( _CONF_ ## PROPERTY )

#define AUTOCONF_LOAD_AS(PROPERTY, CAST) \
    (CAST) AUTOCONF_LOAD(PROPERTY)

// --- stringify ---
#ifndef _str_defined
#define _str_defined 1
    #define str(x) #x
#endif

//
// --- CONFIGURATION BEGIN ---
// 

""")

        for symbol in kconf.unique_defined_syms:
            properties_tot += 1

            name  = symbol.name
            value = symbol.str_value

            f.write(f"// PROPERTY: {name}\n")
            f.write(f"// TYPE:     {symbol.type}\n")

            if symbol.type == kconf.BOOL:
                if value == "y":
                    f.write(f"#define _CONF_{name}\n")
                else:
                    f.write(f"#undef _CONF_{name}\n")
            elif symbol.type == kconf.STRING:
                f.write(f"#define _CONF_{name} str({value})\n")
            elif symbol.type == kconf.INT:
                f.write(f"#define _CONF_{name} {value}\n")
            else:
                write_error(f, name, symbol.type, f"Unknown property type: {symbol.type}")

            f.write("\n\n")

        f.write("#endif // _AUTOCONF_INCLUDED_H\n")

        f.write("//\n")
        f.write("// --- CONFIGURATION END ---\n")
        f.write("//\n\n")

        f.write(f"""
#define AUTOCONF_PROPERTIES_TOT {properties_tot}
#define AUTOCONF_PROPERTIES_ERRORS {errors_occured}
#define AUTOCONF_PROPERTIES_OK {properties_tot - errors_occured}

""")

    print("Configuration tanslation complete.")
    print(f"Audited tot. {properties_tot}, of which {properties_tot - errors_occured} success.")
    
    if(errors_occured > 0):
       print(f"{errors_occured} errors occured. More information on them can be found in the output file as a comments.")

    print("\n")
    print("Have a great day!")

if __name__ == "__main__":
    main()

