#include "ocad.h"
#include "../../core/libfig/fig.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
//#include <sys/types.h>
//#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>


int main(int argc, char **argv){


  if (argc <2) { printf("use: %s <file> > <fig>\n", argv[0]); exit(0); }

  int fd = open(argv[1], O_RDONLY);

  struct ocad_head head;
  read(fd, &head, sizeof(head));

  if (head.ocad_mark!=0x0cad) { printf("Not a OCAD file\n"); exit(0); }

  if ((head.version != 8) && (head.version != 9)) { 
    printf("Unsupported version: %d.%d\n", head.version, head.subversion); 
    exit(0); 
  }


  fig::fig_world W;

  ocad_int pos = head.obj_index_bl;
  while (pos!=0){
    struct ocad8_tobj_index_bl bl;
    lseek(fd, pos, SEEK_SET);
    read(fd, &bl, sizeof(bl));

    for (int i=0;i<256;i++){
      struct ocad8_tobj obj;
      if (bl.table[i].pos == 0) break;
      lseek(fd, bl.table[i].pos, SEEK_SET);
      read(fd, &obj, sizeof(obj));

//      printf("%6d points: %d --\t%d %d %d   %2x %4x %4x %4x %4x %4x %4x\n", 
//        bl.table[i].pos,
//        obj.points, obj.symb, obj.otp, obj.flags, 
//        obj.s1, obj.i3, obj.i4, obj.i5, obj.i6, obj.i7, obj.i8
//      );
//      assert(obj.symb == bl.table[i].symb);

      fig::fig_object O;
      bool unknown=false;
      switch (obj.symb){


        case 1010: O=fig::make_object(  "3 0 0 1 26 26 42 -1 -1 0.000 1 0 0 5"); break; //Горизонталь
        case 1020: O=fig::make_object(  "3 0 0 2 26 26 42 -1 -1 0.000 1 0 0 5"); break; // Утолщ╦нная горизонталь
        case 1030: O=fig::make_object(  "3 0 1 1 26 26 42 -1 -1 4.000 1 0 0 5"); break; // Вспомогательная горизонталь
//1040 1  Бергштрих
//1050 4  отметка высоты
//1060 2  Земляной обрыв
//1070 2  Земляной вал
//1080 2  Маленький земляной вал
        case 1090: O=fig::make_object(  "3 0 0 2 25 7 41 -1 -1 6.000 1 0 0 3"); break; //  Промоина
        case 1100: O=fig::make_object(  "2 1 2 1 25 7 41 -1 -1 2.000 1 1 7 0 0 2"); break; // Маленькая сухая канава
        case 1101: O=fig::make_object(  "2 1 2 2 25 7 41 -1 -1 2.000 1 1 7 0 0 2"); break; // Глубокая сухая канава
        case 1102: O=fig::make_object(  "2 1 2 1 25 7 41 -1 -1 3.000 1 1 7 0 0 2"); break; // Исчезающая сухая канава
//1120 1  Микробугорок
//1121 1  Овальный микробугорок
//1130 1  Маленькая овальная яма
//1132 1  Маленькая яма
//1131 1  Маленькая неглубокая овальн яма
//1133 1  Маленькая неглубокая яма
//1140 1  Микроямка
//1150 1  Воронка
//1160 1  Неровная земля
//1170 1  Особый объект рельефа
//1180 1  Отметка высоты
//1181 4  Отметка высоты - значение
        case 2010: O=fig::make_object( "3 0 0 3 0 0 40 -1 -1 6.000 1 0 0 4"); break; // Непреодолимая скальная стена -1
        case 2011: O=fig::make_object( "3 0 0 3 0 0 40 -1 -1 6.000 1 0 0 4"); break; // Непреодолимая скальная стена -2
        case 2020: O=fig::make_object( "3 1 0 1 0 0 40 -1 20 4.000 1 0 0 6"); break; // Скальные столбы
        case 2030: O=fig::make_object( "3 0 0 2 0 0 40 -1 -1 6.000 1 0 0 4"); break; // Преодолимая скальная стена -1
        case 2031: O=fig::make_object( "3 0 0 2 0 0 40 -1 -1 6.000 1 0 0 4"); break; // Преодолимая скальная стена -2
//2040 1  Скальная воронка
//2050 1  Пещера
//2060 1  Камень
//2070 1  Большой камень
//2080 1  Каменная россыпь -1
//2081 1  Каменная россыпь -2
//2090 1  Группа камней
//2100 1  Каменистая почва
//2110 3  Открытый песок
        case 2120: O=fig::make_object( "2 3 0 0 0 7 49 -1 10 0.000 0 0 7 0 0 5"); break; // Голые скалы
        case 3010: O=fig::make_object("2 3 0 1 22046463  3  65 -1 20 0.000 0 0 -1 0 0 0"); break; // Озеро
        case 3011: O=fig::make_object("2 1 0 1 22046463 7 66 -1 -1 0.000 1 1 0 0 0 0"); break; // Граница пруда
        case 3020: O=fig::make_object("2 3 0 1 22046463  3  65 -1 20 0.000 0 0 -1 0 0 0"); break; // Пруд
//3030 1  Лужа
        case 3041: O=fig::make_object("2 1 0 2 22046463 7 38 -1 -1 0.000 1 1 0 0 0 0"); break; // Узкая непреодолимая река
        case 3042: O=fig::make_object("2 1 0 3 22046463 7 38 -1 -1 0.000 1 1 0 0 0 0"); break; // Непреодолимая река 1
        case 3050: O=fig::make_object("2 1 0 2 22046463 7 38 -1 -1 0.000 1 1 0 0 0 0"); break; // Узкая преодолимая река
        case 3051: O=fig::make_object("2 1 0 3 22046463 7 38 -1 -1 0.000 1 1 0 0 0 0"); break; // Преодолимая река
        case 3060: O=fig::make_object("2 1 0 1 22046463 7 38 -1 -1 0.000 1 1 0 0 0 0"); break; // Ручей
        case 3070: O=fig::make_object("2 1 1 1 22046463 7 38 -1 -1 4.000 1 1 0 0 0 0"); break; // Пересыхающий ручей
        case 3080: O=fig::make_object("2 1 2 1 22046463 7 38 -1 -1 2.000 1 1 0 0 0 0"); break; // Узкое болото
//3090 3  Труднопроходимое болото
//3100 3  Болото
//3101 1  Маленькое болотце
//3110 3  Заболоченность
//3120 1  Колодец
//3130 1  Родник
//3140 1  Особый объект гидрографии

        case 4010: O=fig::make_object("2 3 0 0 0 31 90 -1 20 0.000 0 0 7 0 0 5"); break; // Открытое пространство
        case 4020: O=fig::make_object("2 3 0 0 7 31 90 -1 43 0.000 0 0 7 0 0 5"); break; // Открытое пространство с деревья
        case 4030: O=fig::make_object( "2 3 0 0 7 6 89 -1 20 0.000 0 0 7 0 0 5"); break; //   Неудобь
        case 4040: O=fig::make_object( "2 3 0 0 7 6 89 -1 43 0.000 0 0 7 0 0 5"); break; //   Неудобь с деревьями
        case 4050: O=fig::make_object( "2 3 0 0 7 7 95 -1 20 0.000 0 0 7 0 0 5"); break; //   Белый лес полосой
        case 4051: O=fig::make_object( "2 3 0 0 7 7 84 -1 20 0.000 0 0 7 0 0 5"); break; //   Белый лес в зелени
        case 4052: O=fig::make_object( "2 3 0 0 7 7 83 -1 20 0.000 0 0 7 0 0 5"); break; //   Белый лес в поле
        case 4060: O=fig::make_object( "2 3 0 0 7 14 95 -1 35 0.000 0 0 7 0 0 5"); break; //  Среднепроходимая растительность
        case 4061: O=fig::make_object( "2 3 0 0 7 14 84 -1 35 0.000 0 0 7 0 0 5"); break; //  Среднепр растительность в поле
        case 4070: O=fig::make_object( "2 3 0 0 14 7 95 -1 50 0.000 0 0 7 0 0 5"); break; //  Среднепроходимый подлесок
        case 4080: O=fig::make_object( "2 3 0 0 7 14 95 -1 30 0.000 0 0 7 0 0 5"); break; //  Труднопроходимая растительность
        case 4081: O=fig::make_object( "2 3 0 0 7 14 84 -1 30 0.000 0 0 7 0 0 5"); break; //  Труднопр растительность в поле
//4090 3  Труднопроходимый подлесок
        case 4100: O=fig::make_object( "2 3 0 0 7 14 95 -1 25 0.000 0 0 7 0 0 5"); break; //  Непроходимая растительность
        case 4101: O=fig::make_object( "2 3 0 0 7 14 84 -1 25 0.000 0 0 7 0 0 5"); break; //  Непроход растительность в поле
//4102 3  Завал леса
        case 4103: O=fig::make_object( "2 1 0 2 14 7 50 -1 -1 0.000 1 1 7 0 0 3"); break; //  Узкие кусты
        case 4104: O=fig::make_object( "2 1 0 3 14 7 50 -1 -1 0.000 1 1 7 0 0 3"); break; //  Широкие кусты
        case 7021: O=fig::make_object( "2 1 2 2 14 7 50 -1 -1 3.000 1 1 7 0 0 3"); break; //   узкие кусты с разрывами
//4110 3  Среднепроходим с направлением
//4111 3  Труднопроходим с направлением
//4112 3  Непрходимость с направлением
//4120 3  Сад
        case 4160: O=fig::make_object( "2 1 2 1 0 7 40 -1 -1 2.000 1 0 7 0 0 3"); break; //  Ч╦ткий контур
//4121 1  Дерево в саду
//4122 2  Аллея
//4130 3  Виноградник
        case 4140: O=fig::make_object( "2 1 0 1 0 7 40 -1 -1 2.000 1 0 7 0 0 3"); break; //  Граница возделываемой земли
//4150 3  Возделываемая земля
//9130 3  Цветник
//4180 1  Особый объект растительности -1
//4181 2  Крупная аллея
//4190 1  Особый объект раститльности -2

        case 5010: O=fig::make_object( "3 0 0 10 24 7 30 -1 -1 8.000 0 0 0 3"); break; // Автомагистраль 1
        case 5012: O=fig::make_object( "3 0 0 10 24 7 30 -1 -1 8.000 0 0 0 3"); break; // Автомагистраль 3
        case 5011: O=fig::make_object( "3 0 0 10 24 7 30 -1 -1 8.000 0 0 0 3"); break; // Автомагистраль 2
        case 5030: O=fig::make_object( "3 0 0 6 24 7 30 -1 -1 8.000 0 0 0 3"); break; // Асфальтовая дорога
        case 5020: O=fig::make_object( "3 0 0 8 24 7 30 -1 -1 8.000 0 0 0 3"); break; // Широкое шоссе -1
        case 5022: O=fig::make_object( "3 0 0 8 24 7 30 -1 -1 8.000 0 0 0 3"); break; // Широкое шоссе -3
        case 5021: O=fig::make_object( "3 0 0 8 24 7 30 -1 -1 8.000 0 0 0 3"); break; // Широкое шоссе -2
        case 5023: O=fig::make_object( "3 0 0 8 24 7 30 -1 -1 8.000 0 0 0 3"); break; // Широкое шоссе -4
        case 5031: O=fig::make_object( "3 0 0 4 24 7 30 -1 -1 8.000 0 0 0 3"); break; // Каменная дорога
        case 5034: O=fig::make_object( "3 0 0 4 24 7 30 -1 -1 8.000 0 0 0 3"); break; // Широкий тротуар
        case 5032: O=fig::make_object( "3 0 0 3 24 7 30 -1 -1 8.000 0 0 0 3"); break; // Тротуар
        case 5033: O=fig::make_object( "3 0 0 3 24 7 30 -1 -1 8.000 0 0 0 3"); break; // Тротуар узкий

        case 5040: O=fig::make_object( "3 0 0 2 0 7 30 -1 -1 4.000 0 0 0 3" ); break; // Улучшенная дорога
//5041 2  Лестница
        case 5050: O=fig::make_object( "3 0 1 2 0 7 30 -1 -1 8.000 1 0 0 3"); break; //  Прос╦лочная дорога
        case 5060: O=fig::make_object( "3 0 1 2 0 7 30 -1 -1 5.000 1 0 0 3"); break; //  Тропа
        case 5070: O=fig::make_object( "3 0 1 1 0 7 30 -1 -1 5.000 1 0 0 3"); break; //  Тропинка
        case 5080: O=fig::make_object( "3 0 2 1 0 7 30 -1 -1 3.000 1 0 0 3"); break; //  Исчезающая тропинка
        case 5090: O=fig::make_object( "3 0 1 1 0 7 30 -1 -1 8.000 1 0 0 3"); break; //  Узкая просека
//5110 1  Мостик
//5150 2  Широкая открытая просека
        case 5160: O=fig::make_object( "3 0 0 6 0 7 30 -1 -1 4.000 0 0 0 3"); break; //  Железная дорога1
        case 5151: O=fig::make_object( "3 0 0 6 0 7 30 -1 -1 4.000 0 0 0 3"); break; //  Железная дорога2
//5170 2  Телеграфная линия
//5180 2  Высоковольтная линия
//5181 2  Тоннель/мост
//5191 2  Тоннель
//5192 1  Выход трубы
//5200 2  Каменный забор
//5210 2  Разрушенный каменный забор
//5220 2  Каменный забор выше 1.5м
//5230 2  Ограда изсетки/дерева
//5240 2  Полуразрушенная ограда
//5250 2  Ограда из сетки/дерева выше1.5м
//5251 1  Калитка
//5270 3  Постройка
//8510 3  Постройка
//8520 3  Проход через постройку
//8530 2  Граница постройки
//5271 1  Маленькая постройка
//5280 3  Насел╦нный пункт -1
//5281 3  Насел╦нный пункт -2
//5290 3  Постоянно запрещ для бега район
//5300 3  Территория с покрытием коричнев
//5301 3  Территория с покрытием светлкор
//5302 3  Территория с покрытием серая
//5310 2  Развалины
//5311 1  Маленькие развалины
//5330 2  Огневой рубеж
//5331 1  Огневой рубеж
//5340 1  Могила
//5350 2  Трубопровод
//5360 2  Непреодолимый трубопровод
//5370 1  Высокая башня
//5380 1  Маленькая башня
//5390 1  Каменная пирамида
//5400 1  Кормушка
//5410 1  Особый искуственный объект -1
//5411 1  Кострище
//5420 1  Особый искуственный объект -2
//5421 1  Детская площадка
//5422 1  Мусорные контейнеры
//5424 1  Метка
//8024 4  Подпись к меткам
//6010 1  Старт
//8310 2  Соединительная линия
//6013 1  Старт на схемах
//6014 2  Разметка к старту
//6020 1  Контрольный пункт
//6021 4  Номер контрольного пункта
//6022 1  Контрольный пункт с прорезом
//6023 1  Контрольный пункт для лыж
//6040 1  Финиш
//6050 2  Непреодолимая граница
//6060 1  Проход
//6061 1  Мал. проход
//6070 3  Запрещ╦нный для бега район
//6030 2  Маркированный участок
//6080 3  Опасный район
//6100 1  Медпункт
//6110 1  Пункт питания
//6490 3  Подложка легенд
//7010 2  Лыжня-1
//7020 2  Лыжня-2
//7040 1  Напраывление лыжней
//7030 2  Лыжня-3
//7060 3  Накатанная площадь
//6090 1  Запрещ╦нный маршрут
//8110 2  Скоростная велодорога
//8120 2  Скоростная велотропинка
//8130 2  Медленная велодорога
//8140 2  Медленная велотропинка
//8150 2  Проблемная велодорога
//8160 2  Проблемная велотропинка
//8311 1  Преодолимое велопрепятствие
//8320 1  Непреодолимое велопрепятствие
//8001 4  Текст 0.1
//8002 4  Текст 0.2
//8003 4  Текст 0.2
//8020 4  Подписи к объектам
//8022 4  Подписи к объектам
//8023 4  Подписи к объектам
//8200 2  Линия север-юг
//8201 2  Линия север-юг со стрелкой
//8210 1  Метро
//8211 1  Парковка
//8220 2  Рисовка-1
//8221 1  Рисовка-2
//8800 5  Карточка
//9000 1  Крест совмещения
//9110 3  Трещинки-заливка
//9120 3  Оранжевая заливка
//9160 3  Белый для всего
//9140 3  Фиолетовая заливка
//9170 3  Циан заливка
//9190 3  Т╦мно синяя заливка
//9191 3  Салатовая заливка
//9192 3  Ч╦рная прорезаемая заливка
//9171 3  Сиреневая заливка
//9193 3  Никакой
//9500 1  КП 0
//9501 1  КП -1
//9503 1  КП -2
//9502 1  КП -много
//9510 4  Подробности



        default: unknown=true;
      }
      if (unknown) continue;

      for (int j=0; j<obj.points; j++){
        ocad_int x,y;
        read(fd, &x, sizeof(x));
        read(fd, &y, sizeof(y));
        x = x&0x7FFFFF + (x&0x800000)<<8;
        y = y&0x7FFFFF + (y&0x800000)<<8;
        Point<int> p;
        p.x=int(double(x)/100000*fig::cm2fig);
        p.y=-int(double(y)/100000*fig::cm2fig);
        O.push_back(p);
      }
      W.push_back(O);

    }
    pos = bl.next;
  }
  fig::write(std::cout, W);
}