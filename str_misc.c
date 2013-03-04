#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "str_misc.h"

#define MAX_SUBSTR_LEN  80
#define MAX_STR_LEN    512
static char work_substr[MAX_SUBSTR_LEN];
static char work_str[MAX_STR_LEN];

void do_shift(char *line,int shift)
{
  int n;
  int bytes_to_shift;

  bytes_to_shift = strlen(line);

  if (shift < 0) {
    for (n = 0; n <= bytes_to_shift; n++)
      line[n + shift] = line[n];
  }
  else {
    for (n = bytes_to_shift; (n >= 0); n--)
      line[n + shift] = line[n];
  }
}

int find_substring(char *buf,int *ixpt,char *substring,bool bCaseSens,
  bool bWord)
{
  int ix;
  int str_len;
  int substr_len;
  char *str;
  char *substr;
  int m;
  int n;
  int tries;
  bool bFound;

  ix = *ixpt;

  str_len = strlen(&buf[ix]);
  substr_len = strlen(substring);

  if (substr_len > str_len)
    return false;

  if (bCaseSens) {
    str = &buf[ix];
    substr = substring;
  }
  else {
    if ((str_len > MAX_STR_LEN) || (substr_len > MAX_SUBSTR_LEN))
      return false;

    for (m = 0; m < str_len; m++)
      work_str[m] = tolower(buf[ix+m]);

    for (m = 0; m < substr_len; m++)
      work_substr[m] = tolower(substring[m]);

    str = work_str;
    substr = work_substr;
  }

  tries = str_len - substr_len + 1;

  for (n = 0; n < tries; n++) {
    for (m = 0; m < substr_len; m++) {
      if (str[n+m] != substr[m])
        break;
    }

    if (m == substr_len) {
      bFound = true;

      if (bWord) {
        /* make sure the substring is bounded by whitespace */
        if ((ix + n - 1) && !is_word_delim(buf[ix + n - 1]))
          bFound = false;
        else if (!is_word_delim(buf[ix + n + substr_len]))
          bFound = false;
      }

      if (bFound) {
        *ixpt = ix + n;

        return true;
      }
    }
  }

  return false;
}

int is_word_delim(int chara)
{
  if (!chara || isspace(chara) || (chara == ',') || (chara == '(') ||
    (chara == ')') || (chara == '[') || (chara == ']') || (chara == '\'') ||
    (chara == '"') || (chara == ';') || (chara == '='))
    return true;

  return false;
}

int get_word(char *buf,int *ixpt,char *word,int max_word_len,int *word_len,
  bool bSkipToWhiteSpace)
{
  int ix;
  int save_ix;
  int m;

  ix = *ixpt;

  if (bSkipToWhiteSpace) {
    for ( ; (buf[ix] && !isspace(buf[ix])); ix++)
      ;

    if (!buf[ix])
      return false;
  }

  /* skip to the first non-whitespace character after whitespace */
  for ( ; (buf[ix] && isspace(buf[ix])); ix++)
    ;

  if (!buf[ix])
    return false;

  save_ix = ix;

  for (m = 0; m < max_word_len; ) {
    word[m++] = buf[ix++];

    if (is_word_delim(buf[ix]))
      break;
  }

  if (buf[ix] && !is_word_delim(buf[ix]))
    return false;

  word[m] = 0;
  *ixpt = save_ix;
  *word_len = ix - save_ix;

  return true;
}

int my_strcmpi(char *str1,char *str2)
{
  int n;
  int str1_len;
  int str2_len;

  str1_len = strlen(str1);
  str2_len = strlen(str2);

  if (str1_len != str2_len)
    return 1;

  for (n = 0; n < str1_len; n++) {
    if (tolower(str1[n]) != tolower(str2[n]))
      break;
  }

  if (n < str1_len)
    return 1;

  return 0;
}

int read_straight_translations(char *filename,char *line,int max_line_len,
  struct straight_translation **straight_transl_ptr,
  int *num_straight_transl_ptr)
{
  int m;
  int n;
  FILE *fptr;
  int num_nonblank_lines;
  int num_transl;
  int len;
  bool bError;

  /* go through the file once to determine the number of translations */
  if ((fptr = fopen(filename,"r")) == NULL)
    return false;

  num_nonblank_lines = 0;

  for ( ; ; ) {
    if (fgets(line,max_line_len,fptr) == NULL)
      break;

    if (strlen(line) > 1)
      num_nonblank_lines++;
  }

  num_transl = num_nonblank_lines / LINES_PER_TRANSLATION;

  if (num_transl * LINES_PER_TRANSLATION != num_nonblank_lines) {
    fclose(fptr);
    return false;
  }

  if (((*straight_transl_ptr) = (struct straight_translation *)malloc(
    num_transl * sizeof (struct straight_translation))) == NULL) {
    fclose(fptr);
    return false;
  }

  rewind(fptr);
  bError = false;

  for (n = 0; n < num_transl; n++) {
    m = 0;

    for ( ; ; ) {
      if (fgets(line,max_line_len,fptr) == NULL)
        break;

      len = strlen(line);

      if (len <= 1)
        continue;

      line[len - 1] = 0;

      switch (m) {
        case 0:
          if (((*straight_transl_ptr)[n].old_text = (char *)malloc(len))
            == NULL)
            bError = true;
          else
            strcpy((*straight_transl_ptr)[n].old_text,line);

          break;
        case 1:
          if (((*straight_transl_ptr)[n].new_text = (char *)malloc(len))
            == NULL)
            bError = true;
          else
            strcpy((*straight_transl_ptr)[n].new_text,line);

          break;
      }

      if (bError)
        break;

      m++;

      if (m == LINES_PER_TRANSLATION)
        break;
    }

    if (bError)
      break;
  }

  fclose(fptr);

  if (bError) {
    free(*straight_transl_ptr);

    return false;
  }

  *num_straight_transl_ptr = num_transl;

  return true;
}

int do_straight_translations(char *line,int line_len,int max_line_len,
  int comment_start,int comment_stop,
  struct straight_translation *straight_translations,
  int num_straight_translations)
{
  int n;
  int m;
  int p;
  int q;
  int num_uncommented_fragments;
  int uncommented_start[2];
  int uncommented_stop[2];
  int fragment_len;
  char *fragment;
  char *search_str;
  int search_str_len;
  char *repl_str;
  int repl_str_len;
  bool bFound;
  int shift;
  int retval;

  retval = 0;

  if (!comment_start && (comment_stop == line_len))
    return 0;

  for (n = 0; n < num_straight_translations; n++) {
    if ((comment_start == -1) || (comment_stop == -1)) {
      num_uncommented_fragments = 1;
      uncommented_start[0] = 0;
      uncommented_stop[0] = line_len;
    }
    else if (!comment_start) {
      num_uncommented_fragments = 1;
      uncommented_start[0] = comment_stop;
      uncommented_stop[0] = line_len;
    }
    else {
      num_uncommented_fragments = 2;
      uncommented_start[0] = 0;
      uncommented_stop[0] = comment_start;
      uncommented_start[1] = comment_stop;
      uncommented_stop[1] = line_len;
    }

    search_str = straight_translations[n].old_text;
    search_str_len = strlen(search_str);
    repl_str = straight_translations[n].new_text;
    repl_str_len = strlen(repl_str);

    for (m = 0; m < num_uncommented_fragments; m++) {
      fragment_len = uncommented_stop[m] - uncommented_start[m];

      if (fragment_len >= search_str_len) {
        fragment = &line[uncommented_start[m]];
        p = 0;

        for ( ; ; ) {
          if (fragment_len - p < search_str_len)
            break;

          bFound = false;

          for (q = 0; q < search_str_len; q++) {
            if (fragment[p+q] != search_str[q])
              break;
          }

          if (q == search_str_len) {
            if (is_word_delim(fragment[p+q])) {
              if (!(uncommented_start[m] + p))
                bFound = true;
              else if (is_word_delim(line[uncommented_start[m] + p - 1]))
                bFound = true;
            }
          }

          if (bFound) {
            retval++;

            if (repl_str_len != search_str_len) {
              shift = repl_str_len - search_str_len;

              if (line_len + shift >= max_line_len)
                return -1;

              do_shift(&fragment[p+search_str_len],shift);
              line_len += shift;

              if (comment_start > uncommented_start[m] + p) {
                comment_start += shift;
                comment_stop += shift;
              }
            }

            for (q = 0; q < repl_str_len; q++)
              fragment[p+q] = repl_str[q];

            p += repl_str_len;
          }
          else
            p++;
        }
      }

      if (bFound)
        break;
    }
  }

  return retval;
}
