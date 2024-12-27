#include "s21_decimal.h"

/// @brief Округляет указанное Decimal число до ближайшего целого числа в
/// сторону отрицательной бесконечности.
/// @param dec структура которую хотим изменить
/// @param result стркутра для записи нового числа
/// @return 1 или 0, в зависимости от успешности выполнения
// void s21_dec_cpy(s21_decimal value, s21_decimal *result) {
//     for(int index = 127; index >= 127; index--) {
//         s21_set_bit(result, index, s21_get_bit(value, index));
//     }
// }

int s21_getScale(s21_decimal value) {
  int shift = 16, scale = 0, step = 0;

  while (shift <= 23) {
    int bit = ((value.bits[3] & (1 << shift)) != 0);
    scale += bit * pow(2, step);
    step++;
    shift++;
  }

  return scale;
}

void s21_setScale(s21_decimal *value, int scale) {
  int shift = 16;

  while (shift <= 23) {
    if (scale > 0 && scale % 2 == 1) {
      value->bits[3] |= (1 << shift);
    } else {
      value->bits[3] &= ~(1 << shift);
    }

    scale /= 2;
    shift++;
  }
}

int s21_getSign(s21_decimal *value) { return (value->bits[3] & 1u << 31) != 0; }

// void s21_setSign(s21_decimal *src, int sign) {
//     if (sign == 1) {
//         src->bits[3] |= (1 << 31);
//     } else {
//         src->bits[3] &= ~(1 << 31);
//     }
// }

void s21_decimalInit(s21_decimal *dst) {
  for (int pos = 0; pos < 4; pos++) {
    dst->bits[pos] = 0;
  }
}

// int s21_find_not_equal_bit(s21_decimal value_1, s21_decimal value_2) {
//     int index = 95;

//     if (s21_getScale(value_1) == s21_getScale(value_2)) {
//         while (s21_get_bit(value_1, index) != s21_get_bit(value_2, index) &&
//         index != -1) {
//             index--;
//         }
//     }

//     return index;
// }

int s21_floor(s21_decimal value, s21_decimal *result) {
  int res = 0;

  if (result != NULL && s21_getScale(value) <= 28) {
    s21_decimalInit(result);
    int sign = s21_getSign(&value);
    s21_truncate(value, result);

    if (sign) {
      int index = 0;

      while (s21_get_bit(value, index) == 1) {
        s21_set_bit(result, index, 0);
        index++;
      }

      s21_set_bit(result, index, 1);
    }
  } else {
    res = 1;
  }

  return res;
}

/// @brief Rounds a decimal value to the nearest integer
/// @param dec структура которую хотим изменить
/// @param result стркутра для записи нового числа
/// @return 1 или 0, в зависимости от успешности выполнения
int s21_round(s21_decimal value, s21_decimal *result) {
  int error = 0;

  if (result != NULL) {
    s21_decimalInit(result);
    long double fl_to_round = 0.0;
    long double tmp = 0.0;
    s21_from_decimal_to_double(value, &fl_to_round);

    fl_to_round = round(fl_to_round);

    if (fl_to_round < 0.0) {
      s21_set_bit(result, 127, 1);

      fl_to_round *= -1;
    }

    for (int index = 0; fl_to_round >= 1 && index < 96; index++) {
      fl_to_round = floor(fl_to_round) / 2;
      tmp = fl_to_round - floor(fl_to_round);

      if (tmp > 0.0) {
        s21_set_bit(result, index, 1);
      } else {
        s21_set_bit(result, index, 0);
      }
    }

    s21_setScale(result, 0);
  } else {
    error = 1;
  }

  return error;
}

/// @brief Возвращает целове число, отбрасывая дробную часть
/// @param dec структура которую хотим изменить
/// @param result стркутра для записи нового числа
/// @return 1 или 0, в зависимости от успешности выполнения
int s21_truncate(s21_decimal value, s21_decimal *result) {
  s21_decimalInit(result);

  if (s21_getScale(value)) {
    float dif = 0.0;
    long double fl_to_int = 0.0;
    s21_from_decimal_to_double(value, &fl_to_int);

    fl_to_int = trunc(fl_to_int);

    if (fl_to_int < 0.0) {
      s21_set_bit(result, 127, 1);

      fl_to_int *= -1;
    }

    for (int index = 0; fl_to_int >= 1 && index < 96; index++) {
      fl_to_int = floor(fl_to_int) / 2;
      dif = fl_to_int - floor(fl_to_int);

      if (dif > 0.0) {
        s21_set_bit(result, index, 1);
      } else {
        s21_set_bit(result, index, 0);
      }
    }
    s21_setScale(&value, 0);
  } else {
    for (int index_bits = 0; index_bits < 4; index_bits++) {
      result->bits[index_bits] = value.bits[index_bits];
    }
  }

  return 0;
}

/// @brief Перевод числа в его fabs состояние
/// @param dec структура которую хотим изменить
/// @param result  стркутра для записи нового числа
/// @return 1 или 0, в зависимости от успешности выполнения
int s21_negate(s21_decimal value, s21_decimal *result) {
  int res = 0;
  if (result != NULL) {
    s21_decimalInit(result);
    int sign = s21_getSign(&value);
    *result = value;

    if (sign) {
      s21_set_bit(result, 127, 0);
    } else {
      s21_set_bit(result, 127, 1);
    }
  } else {
    res = 1;
  }

  return res;
}
