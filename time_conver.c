/*****************************************************************************
Copyright (C), 
File name    : time_conver.c
Description  : �ó����ṩ�����ּ�ʱ��ʽ(ͨ��ʱ�������գ�GPSʱ�������)���໥ת��.
Author       : ltp
Version      : 1.0
Date         : 2015-01-28
Others       : 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
1.  �������ʱ(ͨ��ʱ)(commontime)
    ���������Ҳ�ƹ���, �ֱ���������㷺����.�����������һ����146097������ɵ�400������Ϊ����, 1���ƽ������Ϊ365.2425��.
    ���ݸ��������1�걻����Ϊ12����.���ʾʱ��ʱ�����ꡢ�¡��ա�ʱ���֡���ķ���.���ּ�ʱ��ӳ���ڱ仯,���ճ������������,
    ��������,��������ѧ���Ϳ�ѧ����.

2.  ������(julianday)
    ��������һ�ֲ��漰�ꡢ�µȸ���ĳ��������ļ��շ�.������ѧ���ռ��ز��������ǵ�����λ�о���ʹ��.
    ���ַ�������Scaliger��1583�������Ϊ�������ĸ������Զ�����Ϊ������.
    �����յ���㶩�ڹ�Ԫǰ4713��(����ѧ�ϼ�Ϊ-4712��)1��1�ո�������ʱ��ƽ��(����ʱ12:00),
    ��JD0ָ��Ϊ4713 B.C. 1��1��12:00 UT��4713 B.C. 1��2��12:00 UT��24Сʱ��ÿһ�츳����һ��Ψһ������.
    ��������������λ��̫��,��������ѧ���ϻ���1973����ü�������(MJD),�䶨��Ϊ MJD = JD - 2400000.5.
    MJD��Ӧ�������1858��11��17������ʱ0ʱ�� ����1979��10��1����ʱ��������Ϊ2,444,147.5.

3.  GPSʱ(gpstime)
    GPSϵͳ�ڲ������õ�ʱ��ϵͳ��GPSʱ��, GPSʱ��1980��1��6����ҹΪ���,������(һ����������)�����ڵ���������ʾ.

4.  �����(doy)
    ��ν����վ���ָ���Ǵ�ÿ���1��1����ʼ�ۼƵ�����������1��ʼ(��ÿ��1��1�յ������Ϊ1),��2004��5��1�յ������Ϊ122.
    �������Է�������һ��������ʱ��T1��T2���ʱ������
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define ONE_HOUR_MINUTES    (60)
#define ONE_MINUTE_SECONDS  (60)
#define ONE_HOUR_SECONDS    (ONE_HOUR_MINUTES * ONE_MINUTE_SECONDS)
#define ONE_DAY_HOURS       (24)
#define ONE_DAY_MINUTES     (ONE_DAY_HOURS * ONE_HOUR_MINUTES)
#define ONE_DAY_SECONDS     (ONE_DAY_HOURS * ONE_HOUR_SECONDS)

#define TIME_DBG_OPEN       (1) //(memcmp(argv[argc - 1], "dbg", strlen("dbg") == 0))

//ͨ��ʱ
typedef struct common_time_s {
    int   year;
    int   month;
    int   day;
    int   hour;
    int   minute;
    double   second;
} common_time_t;

typedef struct tod_s {
    long sn;        //�������������� 
    double tos;     //������С������ 
} tod_t;

//������
typedef struct julianday_s{
    long day;       //�������� 
    tod_t tod;      //һ���ڵ����� 
} julianday_t;

//��������(��������)
typedef struct new_julianday_s {
    long day;
    tod_t  tod;
} new_julianday_t;

typedef struct tow_s {
    long sn;        //���������� 
    double tos;     //��С������ 
} tow_t;

//GPSʱ
typedef struct gps_time_s {
    int wn;         //���� 
    tow_t tow;      //һ���ڵ����� 
} gps_time_t;

//�����
typedef struct doy_s {
    unsigned short year;
    unsigned short day;
    tod_t tod;
} doy_t;

typedef enum time_type_e {
    TIME_COMMON,
    TIME_JULIAN,
    TIME_GPS,
    TIME_doy_t,
    TIME_MAX
} time_type_t;

typedef enum time_convert_state_e {
    TIME_COMMON_TO_JULIAN,
    TIME_COMMON_TO_GPS,
    TIME_COMMON_TO_doy_t,
    TIME_COMMON_TO_ALL,
    TIME_JULIAN_TO_COMMON,
    TIME_JULIAN_TO_GPS,
    TIME_JULIAN_TO_doy_t,
    TIME_JULIAN_TO_ALL,
    TIME_GPS_TO_COMMON,
    TIME_GPS_TO_JULIAN,
    TIME_GPS_TO_doy_t,
    TIME_GPS_TO_ALL,
    TIME_doy_t_TO_COMMON,
    TIME_doy_t_TO_JULIAN,
    TIME_doy_t_TO_GPS,
    TIME_doy_t_TO_ALL
} time_convert_state_t;

static gps_time_t g_gt;
static common_time_t g_ct;
static julianday_t g_jd;
static doy_t g_doy;

// ȡС������ 
static double time_get_frac(double morigin)
{
    return (morigin - (long)(morigin));
}

//ͨ��ʱ�������յ�ת��
static void time_conver_commontime_to_julianday(common_time_t *pct, julianday_t *pjd)
{
    common_time_t ct;
    double ut;

    memcpy(&ct, pct, sizeof(ct));

    if (ct.year < 1900) {
        if (ct.year < 80) {
            ct.year += 2000;
        } else {
            ct.year += 1900;
        }
    }

    ut = ct.hour + ct.minute / 60.0 + ct.second / 3600.0;

    if (ct.month <= 2) {
        ct.year -= 1;
        ct.month += 12;
    }

    pjd->day = (int)(365.25 * ct.year) + (int)(30.6001 * (ct.month + 1)) + ct.day
        + (int)(ut / 24 + 1720981.5);
    pjd->tod.sn = ((ct.hour + 12) % 24) * 3600 + ct.minute * 60 + (int)ct.second;//����������� 
    pjd->tod.tos = ct.second - (int)ct.second;//���С������ 
}

//�����յ�ͨ��ʱ��ת�� 
static void time_conver_julianday_to_commontime(julianday_t *pjd, common_time_t *pct)
{
    julianday_t jd;
    double x;
    int a, b, c, d, e;

    memcpy(&jd, pjd, sizeof(jd));

    x = jd.day + (jd.tod.sn + jd.tod.tos) / (60.0 * 60.0 * 24);
    a = (int)(x + 0.5);
    b = a + 1537;
    c = (int)((b - 122.1) / 365.25);
    d = (int)(365.25 * c);
    e = (int)((b - d) / 30.6001);

    pct->day = b - d - (int)(30.6001 * e);
    pct->month = e - 1 - 12 * (int)(e / 14);
    pct->year = c - 4715 - (int)((7 + pct->month) / 10);
    pct->hour = (jd.tod.sn / 3600 + 12) % 24;
    pct->minute = (jd.tod.sn % 3600) / 60;
    pct->second = jd.tod.sn % 60 + jd.tod.tos;
}

//�����յ�GPSʱ��ת��
static void time_conver_julianday_to_gpstime(julianday_t *pjd, gps_time_t *pgt)
{
    double x;
    julianday_t jd;

    memcpy(&jd, pjd, sizeof(jd));

    x = jd.day + (jd.tod.sn + jd.tod.tos) / 86400.0;

    pgt->wn = (int)((x - 2444244.5) / 7);
    pgt->tow.sn = (int)(((jd.day - 2444244) % 7 + (jd.tod.sn / (86400.0) - 0.5)) * 86400.0);
    pgt->tow.tos = jd.tod.tos;
}

//GPSʱ�������յ�ת�� 
static void time_conver_gpstime_to_julianday(gps_time_t *pgt, julianday_t *pjd)
{
    gps_time_t gt;

    memcpy(&gt, pgt, sizeof(gt));
    
    pjd->day = (int)(gt.wn * 7 + (double)(gt.tow.sn) / 86400.0 + 2444244.5);
    pjd->tod.sn = (gt.tow.sn + 43200) % 86400;
    pjd->tod.tos = gt.tow.tos;
}

//ͨ��ʱ��GPSʱ��ת��
static void time_conver_commontime_to_gpstime(common_time_t *pct, gps_time_t *pgt)
{
    julianday_t jd;

    time_conver_commontime_to_julianday(pct, &jd);
    time_conver_julianday_to_gpstime(&jd, pgt);
} 

//GPSʱ��ͨ��ʱ��ת��
static void time_conver_gpstime_to_commontime(gps_time_t *pgt, common_time_t *pct)
{
    julianday_t jd;

    time_conver_gpstime_to_julianday(pgt, &jd);
    time_conver_julianday_to_commontime(&jd, pct);
}

//ͨ��ʱ������յ�ת��
static void time_conver_commontime_to_doy(common_time_t *pct, doy_t *pdoy)
{
    common_time_t cto;
    julianday_t jdo;
    julianday_t jd;
    double JD, JDO;

    cto.year = pct->year;
    cto.month = 1;
    cto.day = 1;
    cto.hour = 0;
    cto.minute = 0;
    cto.second = 0;

    time_conver_commontime_to_julianday(&cto, &jdo);
    JDO = jdo.day + (jdo.tod.sn + jdo.tod.tos) / 86400.0;

    time_conver_commontime_to_julianday(pct, &jd);

    JD = jd.day + (jd.tod.sn + jd.tod.tos) / 86400.0;
    pdoy->day = (short)(JD - JDO + 1);
    pdoy->year = pct->year;
    pdoy->tod.sn = (long)(pct->hour * 3600 + pct->minute * 60 + pct->second);
    pdoy->tod.tos = pct->second - (int)(pct->second); 
}

//����յ�ͨ��ʱ��ת��
static void time_conver_doy_to_commontime(doy_t *pdoy, common_time_t *pct)
{
    common_time_t cto;
    julianday_t jdo;
    double JD, JDO;
    long a, b, c, d, e;

    cto.year = pdoy->year;
    cto.month = 1;
    cto.day = 1;
    cto.hour = 0;
    cto.minute = 0;
    cto.second = 0;

    time_conver_commontime_to_julianday(&cto, &jdo);
    JDO = jdo.day + (jdo.tod.sn + jdo.tod.tos) / 86400;
    JD = JDO + pdoy->day + (pdoy->tod.sn + pdoy->tod.tos) / 86400 - 1;

    a = (long)(JD + 0.5);
    b = a + 1537;
    c = (long)((b - 122.1) / 365.25);
    d = (long)(365.25 * c);
    e = (long)((b - d) / 30.6001);

    pct->day = (short)(b - d - (long)(30.6001 * e) + time_get_frac(JD + 0.5));
    pct->month = (short)(e - 1 - 12 * (long)(e / 14));
    pct->year = (short)(c - 4715 - (long)((7 + pct->month) / 10));
    pct->hour = (short)((pdoy->tod.sn + pdoy->tod.tos) / 3600);
    pct->minute = (short)((pdoy->tod.sn + pdoy->tod.tos - pct->hour * 3600) / 60);
    pct->second = pdoy->tod.sn + pdoy->tod.tos - pct->hour * 3600 - pct->minute * 60;
}

//gps������յ�ת��
static void time_conver_gpstime_to_doy(gps_time_t *pgt, doy_t *pdoy)
{
    julianday_t jd;
    common_time_t ct;

    time_conver_gpstime_to_julianday(pgt, &jd);
    time_conver_julianday_to_commontime(&jd, &ct);
    time_conver_commontime_to_doy(&ct, pdoy);
}

//����յ�gps��ת��
static void time_conver_doy_to_gpstime(doy_t *pdoy, gps_time_t *pgt)
{
    common_time_t ct;

    time_conver_doy_to_commontime(pdoy, &ct);
    time_conver_commontime_to_gpstime(&ct, pgt);
}

//�����յ�����յ�ת��
static void time_conver_julianday_to_doy(julianday_t *pjd, doy_t *pdoy)
{
    common_time_t ct;
 
    time_conver_julianday_to_commontime(pjd, &ct);
    time_conver_commontime_to_doy(&ct, pdoy);
} 

//����յ������յ�ת��
static void time_conver_doy_to_julianday(doy_t *pdoy, julianday_t *pjd)
{
    common_time_t ct;

    time_conver_doy_to_commontime(pdoy, &ct);
    time_conver_commontime_to_julianday(&ct, pjd);
}

static void time_print(time_type_t type, void *pt)
{
    switch (type) {
        case TIME_COMMON:
            printf("-->common time:\n");
            printf("ct.year     : %d\n", ((common_time_t *)pt)->year);
            printf("ct.month    : %d\n", ((common_time_t *)pt)->month);
            printf("ct.day      : %d\n", ((common_time_t *)pt)->day);
            printf("ct.hour     : %d\n", ((common_time_t *)pt)->hour);
            printf("ct.minute   : %d\n", ((common_time_t *)pt)->minute);
            printf("ct.seconds  : %lf\n\n", ((common_time_t *)pt)->second);
            break;
        case TIME_JULIAN:
            printf("-->julianday time:\n");
            printf("jd.day      : %d\n", ((julianday_t *)pt)->day);
            printf("jd.tod.sn   : %d\n", ((julianday_t *)pt)->tod.sn);
            printf("jd.tod.tos  : %lf\n\n", ((julianday_t *)pt)->tod.tos);
            break;
        case TIME_GPS:
            printf("-->gps time:\n");
            printf("gps.wn      : %d\n", ((gps_time_t *)pt)->wn);
            printf("gps.tow.sn  : %d\n", ((gps_time_t *)pt)->tow.sn);
            printf("gps.tow.tos : %lf\n\n", ((gps_time_t *)pt)->tow.tos);
            break;
        case TIME_doy_t:
            printf("-->doy time:\n");
            printf("doy.year    : %d\n", ((doy_t *)pt)->year);
            printf("doy.day     : %d\n", ((doy_t *)pt)->day);
            printf("doy.tod.sn  : %d\n", ((doy_t *)pt)->tod.sn);
            printf("doy.tod.tos : %lf\n\n", ((doy_t *)pt)->tod.tos);
            break;
        default:
            break;
    }
}

static int time_get_type_from_name(char *name)
{
    int type = -1;

    if (memcmp(name, "gps", strlen("gps")) == 0) {
        type = TIME_GPS;
    } else if (memcmp(name, "ct", strlen("ct")) == 0) {
        type = TIME_COMMON;
    } else if (memcmp(name, "doy", strlen("doy")) == 0) {
        type = TIME_doy_t;
    } else if (memcmp(name, "jd", strlen("jd")) == 0) {
        type = TIME_JULIAN;
    } else if (memcmp(name, "all", strlen("all")) == 0) {
        type = TIME_MAX;
    } else if (memcmp(name, "quit", strlen("quit")) == 0) {
        type = -1;
    } else if (memcmp(name, "exit", strlen("exit")) == 0) {
        exit(0);
    } else {
        printf("ERROR: The type %s unknown, please input again.\n", name);
        type = -2;
    }

    return type;
}

static int time_convert(void *pt, time_convert_state_t state)
{
    gps_time_t *pgt = &g_gt;
    common_time_t *pct = &g_ct;
    julianday_t *pjd = &g_jd;
    doy_t *pdoy = &g_doy;
    time_type_t type;
    int i;
    void *p;

    switch (state) {
        case TIME_COMMON_TO_JULIAN:
            time_conver_commontime_to_julianday((common_time_t *)pt, pjd);
            type = TIME_JULIAN;
            break;
        case TIME_COMMON_TO_GPS:
            time_conver_commontime_to_gpstime((common_time_t *)pt, pgt);
            type = TIME_GPS;
            break;
        case TIME_COMMON_TO_doy_t:
            time_conver_commontime_to_doy((common_time_t *)pt, pdoy);
            type = TIME_doy_t;
            break;
        case TIME_COMMON_TO_ALL:
            time_conver_commontime_to_julianday((common_time_t *)pt, pjd);
            time_conver_commontime_to_gpstime((common_time_t *)pt, pgt);
            time_conver_commontime_to_doy((common_time_t *)pt, pdoy);
            memcpy(pct, pt, sizeof(common_time_t));
            type = TIME_MAX;
            break;

        case TIME_JULIAN_TO_COMMON:
            time_conver_julianday_to_commontime((julianday_t *)pt, pct);
            type = TIME_COMMON;
            break;
        case TIME_JULIAN_TO_GPS:
            time_conver_julianday_to_gpstime((julianday_t *)pt, pgt);
            type = TIME_GPS;
            break;
        case TIME_JULIAN_TO_doy_t:
            time_conver_julianday_to_doy((julianday_t *)pt, pdoy);
            type = TIME_doy_t;
            break;
        case TIME_JULIAN_TO_ALL:
            time_conver_julianday_to_commontime((julianday_t *)pt, pct);
            time_conver_julianday_to_gpstime((julianday_t *)pt, pgt);
            time_conver_julianday_to_doy((julianday_t *)pt, pdoy);
            memcpy(pjd, pt, sizeof(julianday_t));
            type = TIME_MAX;
            break;

        case TIME_GPS_TO_COMMON:
            time_conver_gpstime_to_commontime((gps_time_t *)pt, pct);
            type = TIME_COMMON;
            break;
        case TIME_GPS_TO_JULIAN:
            time_conver_gpstime_to_julianday((gps_time_t *)pt, pjd);
            type = TIME_JULIAN;
            break;
        case TIME_GPS_TO_doy_t:
            time_conver_gpstime_to_doy((gps_time_t *)pt, pdoy);
            type = TIME_doy_t;
            break;
        case TIME_GPS_TO_ALL:
            time_conver_gpstime_to_commontime((gps_time_t *)pt, pct);
            time_conver_gpstime_to_julianday((gps_time_t *)pt, pjd);
            time_conver_gpstime_to_doy((gps_time_t *)pt, pdoy);
            memcpy(pgt, pt, sizeof(gps_time_t));
            type = TIME_MAX;
            break;

        case TIME_doy_t_TO_COMMON:
            time_conver_doy_to_commontime((doy_t *)pt, pct);
            type = TIME_COMMON;
            break;
        case TIME_doy_t_TO_JULIAN:
            time_conver_doy_to_julianday((doy_t *)pt, pjd);
            type = TIME_JULIAN;
            break;
        case TIME_doy_t_TO_GPS:
            time_conver_doy_to_gpstime((doy_t *)pt, pgt);
            type = TIME_GPS;
            break;
        case TIME_doy_t_TO_ALL:
            time_conver_doy_to_commontime((doy_t *)pt, pct);
            time_conver_doy_to_julianday((doy_t *)pt, pjd);
            time_conver_doy_to_gpstime((doy_t *)pt, pgt);
            memcpy(pdoy, pt, sizeof(doy_t));
            type = TIME_MAX;
            break;
        default:
            return -1;
    }

    for (i = 0; i < TIME_MAX; i++) {
        if (type != TIME_MAX && type != i) {
            continue;
        }

        if (i == TIME_COMMON) {
            p = pct;
        } else if (i == TIME_JULIAN) {
            p = pjd;
        } else if (i == TIME_GPS) {
            p = pgt;
        } else {
            p = pdoy;
        }

        time_print(i, p);
    }

    return 0;
}

static int time_convert_gps(int argc, char *argv[])
{
    int type;
    gps_time_t *pgt = &g_gt;
    time_convert_state_t state;
    char typename[10];
    int rv = 0;
    char yesorno[10];
    bool input_time = true;

    if (pgt->wn) {
        printf("do you want to input gps time again[y/n]: ");
        scanf("%s", yesorno);
        if (memcmp(yesorno, "n", strlen("n")) == 0) {
            input_time = false;
        }
    }

    if (input_time) {
        printf("Please input gpstime: \n");
        printf("week number: ");
        scanf("%d", &pgt->wn);
        printf("Time of week(s): ");
        scanf("%d", &pgt->tow.sn);
        printf("Fractional of seconds: ");
        scanf("%lf", &pgt->tow.tos);
    }

    if (TIME_DBG_OPEN) {
        printf("gps.wn      : %d\n", pgt->wn);
        printf("gps.tow.sn  : %d\n", pgt->tow.sn);
        printf("gps.tow.tos : %lf\n", pgt->tow.tos);
    }

    while (1) {
        printf("\ngps convert to [ct|jd|doy|all|quit|exit]: ");
        scanf("%s", typename);
        type = time_get_type_from_name(typename);
        if (type == -2) {
            continue;
        }

        switch (type) {
            case TIME_COMMON:
                state = TIME_GPS_TO_COMMON;
                break;
            case TIME_JULIAN:
                state = TIME_GPS_TO_JULIAN;
                break;
            case TIME_doy_t:
                state = TIME_GPS_TO_doy_t;
                break;
            case TIME_MAX:
                state = TIME_GPS_TO_ALL;
                break;
            default:
                return -1;
        }

        rv = time_convert(pgt, state);
    }

    return rv;
}

static int time_convert_ct(int argc, char *argv[])
{
    int type;
    common_time_t *pct = &g_ct;
    time_convert_state_t state;
    char typename[10];
    int rv = 0;
    char commtime[64];
    char yesorno[10];
    bool input_time = true;

    if (pct->year) {
        printf("do you want to input common time again[y/n]: ");
        scanf("%s", yesorno);
        if (memcmp(yesorno, "n", strlen("n")) == 0) {
            input_time = false;
        }
    }

    if (input_time) {
        printf("Please input commontime[yyyymmddhhmmss.xx]: ");
        memset(commtime, 0, sizeof(commtime));
        scanf("%s", commtime);
        sscanf(commtime, "%4d%2d%2d%2d%2d%lf", &pct->year, &pct->month, &pct->day, &pct->hour,
            &pct->minute, &pct->second);
    }

    if (TIME_DBG_OPEN) {
        printf("ct.year    : %d\n", pct->year);
        printf("ct.month   : %d\n", pct->month);
        printf("ct.day     : %d\n", pct->day);
        printf("ct.hour    : %d\n", pct->hour);
        printf("ct.minute  : %d\n", pct->minute);
        printf("ct.seconds : %lf\n", pct->second);
    }

    while (1) {
        printf("\ncommontime convert to [gps|jd|doy|all|quit|exit]: ");
        scanf("%s", typename);
        type = time_get_type_from_name(typename);
        if (type == -2) {
            continue;
        }

        switch (type) {
            case TIME_GPS:
                state = TIME_COMMON_TO_GPS;
                break;
            case TIME_JULIAN:
                state = TIME_COMMON_TO_JULIAN;
                break;
            case TIME_doy_t:
                state = TIME_COMMON_TO_doy_t;
                break;
            case TIME_MAX:
                state = TIME_COMMON_TO_ALL;
                break;
            default:
                return -1;
        }

        rv = time_convert(pct, state);
    }

    return rv;
}

static int time_convert_jd(int argc, char *argv[])
{
    int type;
    julianday_t *pjd = &g_jd;
    time_convert_state_t state;
    char typename[10];
    int rv = 0;
    char yesorno[10];
    bool input_time = true;

    if (pjd->tod.sn) {
        printf("do you want to input julianday time again[y/n]: ");
        scanf("%s", yesorno);
        if (memcmp(yesorno, "n", strlen("n")) == 0) {
            input_time = false;
        }
    }

    if (input_time) {
        printf("Please input julianday: \n");
        printf("day: ");
        scanf("%ld", &pjd->day);
        printf("seconds: ");
        scanf("%d", &pjd->tod.sn);
        printf("Fractional of seconds: ");
        scanf("%lf", &pjd->tod.tos);
    }

    if (TIME_DBG_OPEN) {
        printf("jd.day      : %d\n", pjd->day);
        printf("jd.tod.sn   : %d\n", pjd->tod.sn);
        printf("jd.tod.tos  : %lf\n", pjd->tod.tos);
    }

    while (1) {
        printf("\njulianday convert to [ct|gps|doy|all|quit|exit]: ");
        scanf("%s", typename);
        type = time_get_type_from_name(typename);
        if (type == -2) {
            continue;
        }

        switch (type) {
            case TIME_GPS:
                state = TIME_JULIAN_TO_GPS;
                break;
            case TIME_COMMON:
                state = TIME_JULIAN_TO_COMMON;
                break;
            case TIME_doy_t:
                state = TIME_JULIAN_TO_doy_t;
                break;
            case TIME_MAX:
                state = TIME_JULIAN_TO_ALL;
                break;
            default:
                return -1;
        }

        rv = time_convert(pjd, state);
    }

    return rv;
}

static int time_convert_doy(int argc, char *argv[])
{
    int type;
    doy_t *pdoy = &g_doy;
    time_convert_state_t state;
    char typename[10];
    int rv = 0;
    char yesorno[10];
    bool input_time = true;

    if (pdoy->year) {
        printf("do you want to input doy time again[y/n]: ");
        scanf("%s", yesorno);
        if (memcmp(yesorno, "n", strlen("n")) == 0) {
            input_time = false;
        }
    }

    if (input_time) {
        printf("Please input doy: \n");
        printf("year: ");
        scanf("%u", &pdoy->year);
        printf("day: ");
        scanf("%u", &pdoy->day);
        printf("seconds: ");
        scanf("%d", &pdoy->tod.sn);
        printf("Fractional of seconds: ");
        scanf("%lf", &pdoy->tod.tos);
    }

    if (TIME_DBG_OPEN) {
        printf("doy.year     : %d\n", pdoy->year);
        printf("doy.day      : %d\n", pdoy->day);
        printf("doy.tod.sn   : %d\n", pdoy->tod.sn);
        printf("doy.tod.tos  : %lf\n", pdoy->tod.tos);
    }

    while (1) {
        printf("\ndoy convert to [ct|jd|gt|all|quit|exit]: ");
        scanf("%s", typename);
        type = time_get_type_from_name(typename);
        if (type == -2) {
            continue;
        }

        switch (type) {
            case TIME_GPS:
                state = TIME_doy_t_TO_GPS;
                break;
            case TIME_COMMON:
                state = TIME_doy_t_TO_COMMON;
                break;
            case TIME_JULIAN:
                state = TIME_doy_t_TO_JULIAN;
                break;
            case TIME_MAX:
                state = TIME_doy_t_TO_ALL;
                break;
            default:
                return -1;
        }

        rv = time_convert(pdoy, state);
    }

    return rv;
}

#if 0
char *const short_options = "";
struct option long_options[] = {
    {"ct", 0, NULL, 'c'},
    {"doy", 0, NULL, 'd'},
    {"gps", 0, NULL, 'g'},
    {"jd", 0, NULL, 'j'},
};

static void time_show_usage(char *argv)
{
    printf("Usage:\n");
    printf("%s --gps [dbg], convert GpsTime to Other\n", argv);
    printf("%s --jd  [dbg], convert JulianDay to Other\n", argv);
    printf("%s --doy [dbg], convert Doy to Other\n", argv);
    printf("%s --ct  [dbg], convert CommonTime to Other\n", argv);
    printf("\n");
    printf("other: gps|jd|doy|ct|all\n");
    printf("\n");
}
#endif

int main(int argc, char *argv[])  
{
    int rv = 0;
    int char_c;

#if 0
    int arg_cnt = 0;

    while ((char_c = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
        arg_cnt++;
 
        switch (char_c) {
            case 'c':
                rv = time_convert_ct(argc, argv);
                break;
            case 'd':
                rv = time_convert_doy(argc, argv);
                break;
            case 'g':
                rv = time_convert_gps(argc, argv);
                break;
            case 'j':
                rv = time_convert_jd(argc, argv);
                break;
            default:
                rv = -1;
                break;
        }
    }

    if (rv != 0) {
        time_show_usage(argv[0]);
    }
#else
    int type;
    char typename[10];

    while (1) {
        printf("Please input src time[ct|jd|gps|doy|quit|exit]: ");
        scanf("%s", typename);
        type = time_get_type_from_name(typename);
        if (type == -2) {
            continue;
        }

        switch (type) {
            case TIME_GPS:
                rv = time_convert_gps(argc, argv);
                break;
            case TIME_COMMON:
                rv = time_convert_ct(argc, argv);
                break;
            case TIME_doy_t:
                rv = time_convert_doy(argc, argv);
                break;
            case TIME_JULIAN:
                rv = time_convert_jd(argc, argv);
                break;
            default:
                rv = -2;
                break;
        }

        if (rv == -2) {
            break;
        }
    }
#endif

    return rv;
}

