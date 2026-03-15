project('regex', 'c',
    version: '0.1.0',
    default_options: {
        'c_std': 'c23',
        'warning_level': '3',
        'werror': true,
        'strip': true,
    },
)

arguments = [
    '-Wfatal-errors',
]

add_project_arguments(arguments, language: 'c')

sources = files(
    'test.c',
    'main.c',
    'list/list.c',
    'stack/stack.c',
    'queue/queue.c',
    'set/set.c',
    'map/map.c',
    'automaton/common.c',
    'automaton/nfa.c',
    'automaton/dfa.c',
    'automaton/convert.c',
    'automaton/regex.c',
)

executable('regex', sources: sources, install: true)
