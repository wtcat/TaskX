#!/usr/bin/python
# Copyright 2024 wtcat

# Json file
# {
#     "name": "demo",  // The name of State machine
#     "init": "S0",    // The initial state
    
#     "machine" : {
#
#      // State key-words:
#      // "state" : state name
#      // "run"   : run method
#      // "entry" : entry method
#      // "exit"  : exit method
#
#      // If the first child node has the same name as the parent node, 
#      // the current parent node is valid
#
#         "P0": [
#             {
#                 "state": "P0",
#                 "entry": "pr_info(\"Entry P0\");\n",
#                 "exit" : "pr_info(\"Exit P0\");\n"
#             },
#             {
#                 "state": "S0",
#                 "run"  : "pr_info(\"S0\");\n",
#                 "entry": "pr_info(\"Entry\");\n",
#                 "exit" : "pr_info(\"Exit\");\n"
#             },
#             {
#                 "state": "S1"
#             },
#             {
#                 "state": "S2"
#             }
#         ],
#         "P1": [
#             {
#                 "state": "S4"
#             },
#             {
#                 "state": "S5"
#             },
#             {
#                 "state": "S6"
#             }
#         ]
#     }
# }

import json
import sys
import os

sm_c_header = '''
/*
 * Copyright xxxxxxxx
 *
 * Generate by SM-Parser
 */
#define pr_fmt(fmt) "<{name}_sm>: "fmt
#include "basework/generic.h"
#include "basework/log.h"
#include "basework/fsm.h"
'''

sm_c_enum = '''
enum {name}_state {{
{states}
}};
'''

sm_c_context = '''
struct {name}_context {{
    struct fsm_context ctx;

    /* Other state specific data add here */
}};

static struct {name}_context {name}_ctx;
'''

sm_c_method = '''
static void {method}(struct fsm_context *ctx) {{
    //TODO: implement
    //fsm_switch(ctx, &{name}_states[STATE]);
    {body}
}}
'''

sm_c_state_switch = '''
    fsm_swtich(ctx, &{name}_states[{state}]);
'''

sm_c_state_item = '''
    FSM_STATE({state}, {run}, {entry}, {exit}, {parent}),
'''

sm_c_states_table_declare = '''
static const struct fsm_state {name}_states[];
'''

sm_c_states_table = '''
static const struct fsm_state {name}_states[] = {{
{state_items}
}};

'''

sm_c_export_method = '''
int {name}_fsm_init(void) {{
    return fsm_init(&{name}_ctx.ctx, &{name}_states[{init_state}]);
}}

void {name}_fsm_execute(void) {{
    fsm_execute(&{name}_ctx.ctx);
}}

'''

def sm_parser(file):
    with open(file, 'r') as f:
        sm = json.load(f)
        sm_name = sm['name']
        sm_init = sm['init']
        machine = sm['machine']
        group_size = len(machine)

        items_strbuf  = ''
        enum_strbuf   = ''
        method_strbuf = ''
        sm_c_strbuf   = sm_c_header.format(name = sm_name)

        # Parse state machine
        for parent, states in machine.items():
            first_node = True
            k_parent   = 'NULL'

            for sn in states:
                # Get key-value from state
                k_state = sn['state']

                # Run method
                k_run = '{}_run'.format(k_state)
                k_run_body = sn.get('run')
                if k_run_body is None:
                    if first_node:
                        k_run = 'NULL'
                    else:
                        k_run_body = ''

                # Check whether the parent status is valid
                if first_node:
                    if parent == k_state:
                        k_parent = '&{}_states[{}]'.format(sm_name, parent)
                    first_node = False
                
                # Entry method
                k_entry_body = sn.get('entry')
                if k_entry_body is None:
                    k_entry = 'NULL'
                    k_entry_body = ''
                else:
                    k_entry = '{}_entry'.format(k_state)

                # Exit method
                k_exit_body = sn.get('exit')
                if k_exit_body is None:
                    k_exit = 'NULL'
                    k_exit_body = ''
                else:
                    k_exit = '{}_exit'.format(k_state)

                # Generate state item
                k_item = sm_c_state_item.format(
                    state  = k_state,
                    run    = k_run,
                    entry  = k_entry,
                    exit   = k_exit,
                    parent = k_parent
                )

                # Generate state method
                k_method = ''
                if k_run != 'NULL':
                    k_method += sm_c_method.format(
                        method = k_run,
                        name   = sm_name,
                        body   = k_run_body
                    )

                if k_entry != 'NULL':
                    k_method += sm_c_method.format(
                        method = k_entry,
                        name   = sm_name,
                        body   = k_entry_body
                    )

                if k_exit != 'NULL':
                    k_method += sm_c_method.format(
                        method = k_exit,
                        name   = sm_name,
                        body   = k_exit_body
                    )

                # Append to string buffer
                items_strbuf  += k_item
                enum_strbuf   += '\t' + k_state + ',\n'
                method_strbuf += k_method

        # Generate C source code
        sm_c_strbuf += sm_c_enum.format(
            name        = sm_name,
            states      = enum_strbuf
            )
        sm_c_strbuf += sm_c_context.format(
            name        = sm_name
            )
        sm_c_strbuf += sm_c_states_table_declare.format(
            name        = sm_name
            )
        sm_c_strbuf += method_strbuf
        sm_c_strbuf += sm_c_states_table.format(
            name        = sm_name,
            state_items = items_strbuf
            )
        sm_c_strbuf += sm_c_export_method.format(
            name        = sm_name,
            init_state  = sm_init
        )

        print("C-test:\n", sm_c_strbuf)

        # Flush to file
        filename, extname = os.path.splitext(file)
        outfile = filename + '.c'

        print('output file: ', outfile)
        with open(outfile, 'w') as fout:
            fout.write(sm_c_strbuf)


def main(argv):
    if len(argv) < 2:
        print('Usage: sm_pareser.py input_file')
        return

    sm_parser(argv[1])

if __name__ == "__main__":
    main(sys.argv)