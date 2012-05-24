/*
 * ===========================================================================
 *
 *       Filename:  soduku.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2012年05月21日 16时16分08秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  nuoerlz (nuoliu), nuoerlz@gmail.com
 *        Company:  mhtt
 *
 *      CopyRight:  Reserve
 *
 * ===========================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "soduku.h"


// #define NAL_DEBUG
#define SODK_CAL_MAX_COUNT      1000000


enum em_sodkAlg {
    SODK_ALG_RANDOM,
    SODK_ALG_INTERVAL,
    SODK_ALG_SNAKE,
    SODK_ALG_ORDER,
    SODK_ALG_COUNT
};

typedef struct tagSodkDigInfos {
    enum em_sodkGrade   mGrade;
    enum em_sodkAlg     mAlg;
    int                 mAreaHigh;
    int                 mAreaLow;
    int                 mRowHigh;
    int                 mRowLow;
    int                 mColHigh;
    int                 mColLow;
} SodkDigInfos;



static unsigned int Rand_int(unsigned int a, unsigned int b);
static void Rand_randXN(int *X, int n);

static void sodk_ST_resetBanDig();
static void sodk_ST_initHerp();

static int  sodk_ST_constraintRowCol(int *tab, int idx,
        const SodkDigInfos *dig);
static int  sodk_ST_canDig(int *tab, int idx);
// static int  sodk_ST_verifySixs();
static int  sodk_ST_oriIsOK(int *tab);
static int  sodk_ST_isOK(const int *tab, int idx);

static int  sodk_ST_cal(int *tab);
static int  sodk_ST_algRandom(int *tab, const SodkDigInfos *dig);



static long long    g_calCount;
static int          g_curTabIdx;
static int          g_sodkHerp[SODK_ROWS * SODK_COLS];
static int          g_sodkBanDig[SODK_ROWS * SODK_COLS];
static const int    g_sodkSixs[9][9] = {
    { 0, 1, 2, 9, 10, 11, 18, 19, 20 },
    { 3, 4, 5, 12, 13, 14, 21, 22, 23 },
    { 6, 7, 8, 15, 16, 17, 24, 25, 26 },
    { 27, 28, 29, 36, 37, 38, 45, 46, 47 },
    { 30, 31, 32, 39, 40, 41, 48, 49, 50 },
    { 33, 34, 35, 42, 43, 44, 51, 52, 53 },
    { 54, 55, 56, 63, 64, 65, 72, 73, 74 },
    { 57, 58, 59, 66, 67, 68, 75, 76, 77 },
    { 60, 61, 62, 69, 70, 71, 78, 79, 80 },
};
static const SodkDigInfos g_sodkDigs[SODK_GRADE_COUNT] = {
    { SODK_GRADE_LOW,     SODK_ALG_RANDOM,   80, 55, 9, 6, 9, 6 },
    { SODK_GRADE_PRIMARY, SODK_ALG_RANDOM,   69, 45, 9, 4, 9, 4 },
    { SODK_GRADE_MIDDLE,  SODK_ALG_INTERVAL, 58, 47, 9, 3, 9, 3 },
    { SODK_GRADE_HIGH,    SODK_ALG_SNAKE,    36, 47, 9, 1, 9, 1 },
    { SODK_GRADE_ASHES,   SODK_ALG_ORDER,    25, 36, 9, 0, 9, 0 },
};



/*==========================================================================*
 * @Description:
 *      返回一个整形随机数, 值介于a和b之间, 包括a和b;
 *      return a random number, it's value is in [a, b].
 *
 * @Param   a
 * @Param   b
 *
 * @Returns:
 *      return a random number, it's value is in [a, b].
 *
 *==========================================================================*/
static unsigned int Rand_int(unsigned int a, unsigned int b)
{
    assert( a <= b);

    return a + rand() % (b + 1 - a);
}

/*==========================================================================*
 * @Description:
 *      返回一个整形随机数组, 大小为n, 元素是唯一的, 且其元素值介于0和n-1之间.
 *
 * @Param   n
 *
 *==========================================================================*/
static void Rand_randXN(int *X, int n)
{
    assert(n >= 1);

    int i, a, t;

    for ( i = 0; i < n; i++ )
        X[i] = i;

    // srand((unsigned int)time(NULL));
    for ( i = 0; i < n; i++ )
    {
        a = Rand_int(i, n - 1);
        t = X[a];
        X[a] = X[i];
        X[i] = t;
    }
}

#if 0
static int sodk_ST_verifySixs()
{
    int i, j, m, n;

    for ( i = 0; i < 9; i++ )
    {
        for ( j = 0; j < 9; j++ )
        {
            int iTmp = g_sodkSixs[i][j];
            int idx1 = i * 9 + j;
            assert(iTmp >= 0 && iTmp <= 80);
            for ( m = 0; m < 9; m++ )
            {
                for ( n = 0; n < 9; n++ )
                {
                    int idx2 = m * 9 + n;
                    if ( idx1 != idx2 && iTmp == g_sodkSixs[m][n] )
                        return 0;
                }
            }
        }
    }

    return 1;
}
#endif

static void sodk_ST_resetBanDig()
{
    int i;
    for ( i = 0; i < SODK_ROWS * SODK_COLS; i++ )
        g_sodkBanDig[i] = SODK_DIGI_CANDIG;
}

static int sodk_ST_oriIsOK(int *tab)
{
    int i;
    for ( i = 0; i < SODK_ROWS * SODK_COLS; i++ )
        if ( sodk_DE_isValidDigi(tab[i]) && !sodk_ST_isOK(tab, i) )
            return 0;

    return 1;
}

static int sodk_ST_isOK(const int *tab, int idx)
{
    int i, j, k;
    int row = idx / SODK_ROWS;
    int col = idx % SODK_ROWS;

    g_calCount++;

    for ( i = row*SODK_ROWS; i < row*SODK_ROWS+SODK_COLS; i++ )
        if ( i != idx && sodk_DE_isValidDigi(tab[i]) && tab[idx] == tab[i] )
            return 0;


    for ( i = col; i < SODK_COLS*SODK_ROWS; i += SODK_ROWS )
        if ( i != idx && sodk_DE_isValidDigi(tab[i]) && tab[idx] == tab[i] )
            return 0;


    for ( i = 0; i < SODK_ROWS; i++ )
        for ( j = 0; j < SODK_COLS; j++ )
            if ( idx == g_sodkSixs[i][j] )
                goto _FINDED_IDX;

_FINDED_IDX:
    assert( i != SODK_ROWS && j != SODK_COLS );
    for ( k = 0; k < SODK_ROWS; k++ )
        if ( k != j && tab[idx] == tab[g_sodkSixs[i][k]] )
            return 0;

    return 1;
}

static void sodk_ST_initTab(int *tab)
{
    int i, j;

    for ( i = 0; i < SODK_ROWS; i++ )
    {
        for ( j = 0; j < SODK_COLS; j++ )
        {
            tab[i*SODK_ROWS+j] = SODK_DIGI_ORI;
        }
    }
}

static void sodk_ST_initHerp()
{
    int i, j;

    for ( i = 0; i < SODK_ROWS; i++ )
    {
        for ( j = 0; j < SODK_COLS; j++ )
        {
            g_sodkHerp[i*SODK_ROWS+j] = SODK_DIGI_LOW;
        }
    }
}

static int sodk_ST_cal(int *tab)
{
    assert(tab != NULL);
    int i, j, k, idx;
    int iFirstSpace = -1;

    sodk_ST_initHerp();

    g_calCount = 0;
    for ( i = 0; i < SODK_ROWS; i++ )
        for ( j = 0; j < SODK_COLS; j++ )
            if ( !sodk_DE_isValidDigi(tab[i*SODK_ROWS+j]) )
            {
                iFirstSpace = i*SODK_ROWS+j;
                goto _FINDED_FIRST_SPACE;
            }
_FINDED_FIRST_SPACE:
    assert(iFirstSpace != -1);

    for ( i = 0; i < SODK_ROWS * SODK_COLS; i++ )
    {
        idx = i;
        if ( !sodk_DE_isValidDigi(tab[idx]) )
        {
            for ( k = g_sodkHerp[idx]; k <= SODK_DIGI_HIGH; k++ )
            {
                tab[idx] = k;
                g_sodkHerp[idx] = k + 1;
                if ( sodk_ST_isOK(tab, idx) )
                {
                    break;
                }
            }
            if ( k > SODK_DIGI_HIGH && idx > iFirstSpace )
            {
                g_sodkHerp[idx] = SODK_DIGI_LOW;
                tab[idx-1] = tab[idx] = SODK_DIGI_ORI;
                i -= 2;
            }
            else if ( k > SODK_DIGI_HIGH && idx <= iFirstSpace )
            {
#ifdef NAL_DEBUG
                printf("NAL && iF=%d, idx=%d\n", iFirstSpace, idx);
                st_print(tab);
                assert(0);
#endif
                return 0;
            }
        }
        if ( g_calCount > SODK_CAL_MAX_COUNT ) // MAGIC
            return 0;
    }

    return 1;
}

static int sodk_ST_constraintRowCol(int *tab, int idx,
        const SodkDigInfos *dig)
{
    int i;
    int row = idx / SODK_ROWS;
    int col = idx % SODK_COLS;
    int nrRowOri = 0, nrColOri = 0;

    if ( g_sodkBanDig[idx] == SODK_DIGI_BANDIG )
        return 0;

    if ( !sodk_DE_isValidDigi(tab[idx]) )
        return 0;

    for ( i = row*SODK_ROWS; i < row*SODK_ROWS+SODK_COLS; i++ )
        if ( sodk_DE_isValidDigi(tab[i]) )
            nrRowOri++;

    for ( i = col; i < SODK_COLS*SODK_ROWS; i += SODK_ROWS )
        if ( sodk_DE_isValidDigi(tab[i]) )
            nrColOri++;

    if ( nrRowOri < dig->mRowLow || nrColOri < dig->mColLow )
        return 0;

    return 1;
}

static int sodk_ST_canDig(int *tab, int idx)
{
    int i;
    int tabTmp[SODK_ROWS*SODK_COLS];
    int iTmp = tab[idx];

    for ( i = 0; i < SODK_ROWS * SODK_COLS; i++ )
        tabTmp[i] = tab[i];

    // tabTmp[idx] = 0;

    for ( i = SODK_DIGI_LOW; i <= SODK_DIGI_HIGH; i++ )
    {
        if ( i != iTmp )
        {
            tabTmp[idx] = i;
            if ( sodk_ST_isOK(tabTmp, idx) )
            {
                if ( sodk_ST_cal(tabTmp) )
                    return 0;
            }
        }
    }

    return 1;
}

static int sodk_ST_algRandom(int *tab, const SodkDigInfos *dig)
{
    int i, k, rIdx;
    int rA[SODK_ROWS*SODK_COLS];
    int nrKnown = SODK_ROWS * SODK_COLS;

    sodk_ST_resetBanDig();

    Rand_randXN(rA, SODK_ROWS*SODK_COLS);
    for ( i = nrKnown, k = 0; i > dig->mAreaLow; )
    {
        do {
            if ( k >= SODK_ROWS * SODK_COLS )
                return 0;
            rIdx = rA[k++];
        } while ( !sodk_ST_constraintRowCol(tab, rIdx, dig) );
        // printf("NAL 2 +======+ i=%d\n", i);

        if ( sodk_ST_canDig(tab, rIdx) )
        {
            printf("NAL 4 +===========+ i=%d\n", i);
            tab[rIdx] = SODK_DIGI_DIG;
            i--;
        }
        else
        {
            printf("NAL 5 +===========+ i=%d\n", i);
            g_sodkBanDig[rIdx] = SODK_DIGI_BANDIG;
        }
    }

    return 1;
}


int sodk_create(int *tab)
{
    assert(tab != NULL);

    int k;

#if 0
    assert(sodk_ST_verifySixs());
#endif

    do {
        do { 
            sodk_ST_initTab(tab);
            sodk_ST_initHerp();
            for ( k = 0; k < 11; k++ ) /* 11 is a MAGIC Number */
            {
                int rIdx = Rand_int(0, SODK_ROWS*SODK_COLS-1);
                int rVal = Rand_int(SODK_DIGI_LOW, SODK_DIGI_HIGH);
                tab[rIdx] = rVal;
            }
        } while ( ! sodk_ST_oriIsOK(tab) );
        g_curTabIdx++;
        // printf("HHHHHHHHHHHHHHHHHHHHHHHH g_curTabIdx=%d\n", g_curTabIdx);
    } while ( ! sodk_ST_cal(tab) );

    return 1;
}

int sodk_verification(int *tab)
{
    int i;
    for ( i = 0; i < SODK_ROWS * SODK_COLS; i++ )
    {
        if ( ! sodk_DE_isValidDigi(tab[i]) )
            return 0;
        if ( ! sodk_ST_isOK(tab, i) )
            return 0;
    }
    return 1;
}

int sodk_dig(int *tab, enum em_sodkGrade egd)
{
    assert(tab != NULL && egd >= 0 && egd < SODK_GRADE_COUNT);

    int                ret;
    const SodkDigInfos *pDigInfo = &g_sodkDigs[egd];

    assert(pDigInfo->mGrade == egd);
    switch ( pDigInfo->mAlg )
    {
        case SODK_ALG_RANDOM:
            ret = sodk_ST_algRandom(tab, pDigInfo);
            break;
        default:
            assert(0);
            break;
    }

    return ret;
}
