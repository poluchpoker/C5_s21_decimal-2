#include "s21_decimal.h"

/// @brief  Перевод из int в decimal
/// @param src
/// @param dst
/// @return 0 - OK 1 - ошибка конвертации
int s21_from_int_to_decimal(int src, s21_decimal *dst) {
  convertors result = OK;

  if (dst) {
    s21_decimalInit(dst);
    if (src < 0) {
      dst->bits[3] |= 1 << 31;
      src = ~src;
      src++;
    }
    if (src > INT_MAX) {
      result = ERROR;
    } else {
      dst->bits[0] = src;
      result = OK;
    }
  } else {
    result = ERROR;
  }

  return result;
}

int s21_from_float_to_decimal(float src, s21_decimal *dst) {
  convertors res = OK;
  s21_decimalInit(dst);

  if (isinf(src) || isnan(src)) {
    res = ERROR;
  } else {
    if (src != 0) {
      int sign = *(int *)&src >> 31;
      int exp = ((*(int *)&src & ~0x80000000) >> 23) - 127;
      double tmp = (double)fabs(src);
      int off = 0;
      for (; off < 28 && (int)tmp / (int)pow(2, 21) == 0; tmp *= 10, off++) {
      }
      tmp = round(tmp);
      if (off <= 28 && (exp > -94 && exp < 96)) {
        floatbits mant = {0};
        tmp = (float)tmp;
        for (; fmod(tmp, 10) == 0 && off > 0; off--, tmp /= 10) {
        }
        mant.fl = tmp;
        exp = ((*(int *)&mant.fl & ~0x80000000) >> 23) - 127;
        dst->bits[exp / 32] |= 1 << exp % 32;
        for (int pos = exp - 1, pos1 = 22; pos1 >= 0; pos--, pos1--)
          if ((mant.ui & (1 << pos1)) != 0)
            dst->bits[pos / 32] |= 1 << pos % 32;
        dst->bits[3] = (sign << 31) | (off << 16);
      }
    }
  }

  return res;
}
/// @brief Перевод из decimal во float
/// @param src
/// @param dst
/// @return 0 - OK 1 - ошибка конвертации
int s21_from_decimal_to_float(s21_decimal src, float *dst) {
  convertors result_errors = ERROR;
  if (dst) {
    *dst = 0;
    double tmp = 0;
    for (int pos = 0; pos < 96; pos++) {
      if (s21_get_bit(src, pos)) {
        tmp += pow(2.0, pos);
      }
    }
    int scale = s21_getScale(src);
    while (scale != 0) {
      tmp /= 10;
      scale--;
    }
    if (s21_getSign(&src)) {
      tmp = -tmp;
    }
    *dst = tmp;
    result_errors = OK;
  }

  return result_errors;
}

/// @brief  Перевод из decimal в int
/// @param src
/// @param dst
/// @return

int s21_from_decimal_to_int(s21_decimal src, int *dst) {
  convertors res = OK;
  int scale = s21_getScale(src);

  if (src.bits[1] || src.bits[2]) {
    res = ERROR;
  } else {
    *dst = src.bits[0];
    if (scale > 0 && scale <= 28) {
      *dst /= pow(10, scale);
    }
  }

  if (s21_get_bit(src, 127)) *dst = (-1) * *dst;

  return res;
}

/// @brief  Перевод из decimal в double
/// @param src
/// @param dst
/// @return

int s21_from_decimal_to_double(s21_decimal src, long double *result) {
  long double res = (double)*result;

  for (int index = 0; index < 95; index++) {
    res += s21_get_bit(src, index) * pow(2, index);
  }

  res = res * pow(10, -s21_getScale(src));

  if (s21_get_bit(src, 127)) res *= -1;
  *result = res;

  return 0;
}