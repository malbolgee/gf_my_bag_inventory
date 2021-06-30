import json
from re import match
from re import search
from time import sleep

t_monster = open(r"./ini_files/T_Monster.ini",
                 'r', encoding='ISO 8859-15')
c_monster = open(r"./ini_files/C_Monster.ini",
                 'r', encoding='Big5', errors='ignore')

# Name, HP, MP
monsters = {}


def parse_monster_ini(c_monster_arg, t_monster_arg):
    parse_t_monster(t_monster_arg)
    parse_c_monster(c_monster_arg)


def parse_t_monster(arg_file):

    while (True):

        line = arg_file.readline().rstrip('\n')

        if not line:
            break

        if (match(r'^\d{5}\|.*', line)):

            lineSplit = list(filter(None, line.split('|')))
            monsterCode = lineSplit[0]

            monsterName = ''
            
            # info is only one line
            if (line[-1] == '|'):
                monsterName = lineSplit[1]
            else:
                
                # info is broken into two lines
                monsterTitle = lineSplit[1]
                line = arg_file.readline().rstrip('\n')
                monsterName = monsterTitle + ' ' + line[:-1]

            monsters[monsterCode] = [monsterName]


def parse_c_monster(arg_file):

    while (True):

        line = arg_file.readline().rstrip('\n')

        if not line:
            break

        try:

            if match(r'\d{5}\|.*', line):

                hasMap = search('map', line)
                splitLine = list(filter(None, line.split('|')))
                monsterCode = splitLine[0]
                
                # info is in only one line
                if (line[-1] == '|'):
                    
                    # grab the hp and mp values
                    monsters[monsterCode].append(splitLine[7 if hasMap else 6])
                    monsters[monsterCode].append(splitLine[8 if hasMap else 7])
                else:
                    
                    # info is in the next line
                    line = arg_file.readline().rstrip('\n')
                    hasMap = search('map', line)
                    splitLine = list(filter(None, line.split('|')))
                    monsters[monsterCode].append(splitLine[5 if hasMap else 4])
                    monsters[monsterCode].append(splitLine[6 if hasMap else 5])

        except KeyError:
            print('keyError: ' + monsterCode)
        except IndexError:
            print('indexError: ' + monsterCode)


parse_monster_ini(c_monster, t_monster)

out_file = open(r"./out.json",
                'w', encoding='utf-8')

# Write into a json file
for key, item in monsters.items():

    try:
        out_file.write(json.dumps({'id': int(key), 'name': item[0], 'hp': int(
            float(item[1])), 'mp': int(float(item[2]))}, ensure_ascii=False))
        out_file.write('\n')
    except IndexError:
        pass
