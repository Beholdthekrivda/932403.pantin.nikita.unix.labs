#!/bin/bash

set -e

cleanup() {
    local rc=$?
    trap - EXIT
    rm -rf -- "$temp_dir"
    exit $rc
}

trap cleanup EXIT HUP INT QUIT PIPE TERM

# trap нужен для обработки сигналов при выходе из скрипта
# EXIT, INT, TERM - сигналы, при получении которых будет вызвана функция cleanup
# EXIT - генерируется при выходе из скрипта или оболочки bash (нормальном или аварийном)
# INT - прерывание (например, Ctrl+C)
# TERM - cтандартный сигнал для запроса завершения процесса

file="${PWD}/$1"

extension="${1##*.}"

temp_dir=$(mktemp -d "${PWD}"/temp.XXXXXX)

if [ "$extension" == "cpp" ] || [ "$extension" == "c" ]; then
    result=$(awk -F '&Output:' '{print $2}' "$file" | xargs)
    # xargs удаляет пробелы в начале и конце строки, хотя используется он для того, чтобы подставить данные в аргументы команды, которые являются результатом другой программы
    # обычно он идет сразу после конвейера |, например, cat file.txt | xargs rm -f
    # awk ищет строку с &Output: и выводит всё, что после неё 
    if [ ! -z "$result" ]; then 
        # -z проверяет, что строка не пустая

        cp "$file" "$temp_dir/"
        cd "$temp_dir"

        if g++ "$file" -o "$result"; then
            echo "Компиляция успешна"
        else
            echo "Ошибка компиляции"
            exit 1
        fi

    	cp "$result" "${PWD}"/..
    
   	echo "Запуск программы..."
    	"./$result"  
    else
        echo "не найден флаг"
    fi

elif [ "$extension" == "tex" ]; then
    cp "$file" "$temp_dir/"
    cd "$temp_dir"
		
    if pdflatex "$temp_dir/$(basename "$file")"; then
	echo "Компиляция успешна"
    else
	echo "Ошибка компиляции"
	exit 1
    fi

    cp "$temp_dir/$(basename "${file%.*}.pdf")" "${PWD}"/..
else 
    echo "Файл не является cpp/c или tex"
fi
