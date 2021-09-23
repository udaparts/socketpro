package SPA;

//java.time.LocalDateTime will be supported in the future
class UDateTime {

    public long time = 0;

    public UDateTime(long t) {
        time = t;
    }

    public UDateTime(java.util.Date dt) {
        set(dt, 0);
    }

    public UDateTime(java.util.Date dt, int ns) {
        set(dt, ns);
    }

    public void set(java.util.Date dt) {
        set(dt, 0);
    }

    public final void set(java.util.Date dt, int ns) throws IllegalArgumentException {
        if (dt == null) {
            throw new IllegalArgumentException("Date dt can not be null");
        }
        if (ns < 0 || ns >= 1000000) {
            throw new IllegalArgumentException("Parameter ns cannot have milliseconds");
        }
        java.util.Calendar calendar = new java.util.GregorianCalendar();
        calendar.setTime(dt);
        int year = calendar.get(java.util.Calendar.YEAR);
        int month = calendar.get(java.util.Calendar.MONTH);
        int day = calendar.get(java.util.Calendar.DAY_OF_MONTH);
        if (year == 2 && month == 11 && day == 31) {
            //treated as time only
            time = 0;
        } else {
            if (year >= 1900) {
                time = year - 1900;
            } else {
                time = 1; //negative, before 1900-01-01
                //time <<= 13; //year 13bits
                time <<= 13;
                time += 1900 - year;
            }
            time <<= 4; //month 4bits
            time += month;
            time <<= 5; //day 5bits
            time += day;
        }
        time <<= 5; //hour 5bits
        time += calendar.get(java.util.Calendar.HOUR_OF_DAY);
        time <<= 6; //minute 6bits
        time += calendar.get(java.util.Calendar.MINUTE);
        time <<= 6; //second 6bits
        time += calendar.get(java.util.Calendar.SECOND);
        time <<= 24; //ticks 24bits
        long ticks = calendar.get(java.util.Calendar.MILLISECOND);
        ticks *= 10000;
        ticks += ns / 100;
        time += ticks;
    }
    private static final int MICRO_SECONDS = 0xfffff; //20 bits

    public java.util.Date get() {
        long dt = this.time;
        int ns100 = (int) (dt & 0xffffff); //24bits
        dt >>= 24;
        int second = (int) (dt & 0x3f); //6bits
        dt >>= 6;
        int minute = (int) (dt & 0x3f); //6bits
        dt >>= 6;
        int hour = (int) (dt & 0x1f); //5bits
        dt >>= 5;
        int day = (int) (dt & 0x1f); //5bits
        dt >>= 5;

        //0 - 11 instead of 1 - 12
        int month = (int) (dt & 0xf); //4bits
        dt >>= 4;

        //8191 == 0x1fff, From BC 6291 (8191 - 1900) to AD 9991 (1900 + 8191)
        int year = (int) (dt & 0x1fff); //13bits
        dt >>= 13;

        //It will be 1 if date time is earlier than 1900-01-01
        int neg = (int) dt;
        if (year == 0 && day == 0 && month == 0) {
            year = -1900; //treated as time-only
        }
        if (neg != 0) {
            year = -year;
        }
        int ms = ns100 / 10000;
        java.util.GregorianCalendar gc = new java.util.GregorianCalendar(year + 1900, month, day, hour, minute, second);
        java.util.Date date = gc.getTime();
        if (ms != 0) {
            dt = date.getTime();
            dt += ms;
            date.setTime(dt);
        }
        return date;
    }
}
