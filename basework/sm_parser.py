#!/usr/bin/python

import json

sm_c_enum = '''
enum {name}_state {{
    {states}
}};

'''

sm_c_context = '''
struct {name}_context {{
    struct fsm_context ctx;

    /* Other state specific data add here */
}} {name}_ctx;

'''

sm_c_method = '''
static void {method}(struct fsm_context *ctx) {{
    //TODO: implement
}}

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
    fsm_init(&{name}_ctx.ctx, {name}_states[{init_state}]);
    return 0;
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

        sm_c_state_items_strbuf  = ''
        sm_c_state_enum_strbuf   = ''
        sm_c_state_method_strbuf = ''
        sm_c_strbuf              = ''

        #Parse state machine
        for parent, states in machine.items():
            if group_size > 1:
                sm_c_state_enum_strbuf += '\t' + parent + ',\n'
                k_parent = parent
            else:
                k_parent = 'NULL'

            for sn in states:
                # Get key-value from state
                k_state = sn['state']
                k_run   = '{}_run'.format(k_state)
                k_entry = 'NULL'
                k_exit  = 'NULL'

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
                        method = k_run
                    )

                if k_entry != 'NULL':
                    k_method += sm_c_method.format(
                        method = k_entry
                    )

                if k_exit != 'NULL':
                    k_method += sm_c_method.format(
                        method = k_exit
                    )

                # Append to string buffer
                sm_c_state_items_strbuf  += k_item
                sm_c_state_enum_strbuf   += '\t' + k_state + ',\n'
                sm_c_state_method_strbuf += k_method

                # print(sn, k_item)

        # Generate C source code

        sm_c_strbuf = sm_c_enum.format(
            name        = sm_name,
            states      = sm_c_state_enum_strbuf
            )
        sm_c_strbuf += sm_c_context.format(
            name        = sm_name
            )
        sm_c_strbuf += sm_c_states_table_declare.format(
            name        = sm_name
            )
        sm_c_strbuf += sm_c_state_method_strbuf
        sm_c_strbuf += sm_c_states_table.format(
            name        = sm_name,
            state_items = sm_c_state_items_strbuf
            )
        sm_c_strbuf += sm_c_export_method.format(
            name        = sm_name,
            init_state  = sm_init
        )

        print("C-test:\n", sm_c_strbuf)


def main():
    sm_parser('sm.json')

if __name__ == "__main__":
    main()