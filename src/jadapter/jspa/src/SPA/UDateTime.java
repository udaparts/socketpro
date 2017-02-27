package SPA;

//java.time.LocalDateTime will be supported in the future
class UDateTime {

    public long time;

    public UDateTime(long t) {
        time = t;
    }

    public UDateTime(java.util.Date dt) {
        set(dt, (short) 0);
    }

    public UDateTime(java.util.Date dt, short us) {
        set(dt, us);
    }

    public void set(java.util.Date dt) {
        set(dt, (short) 0);
    }

    public void set(java.util.Date dt, short us) {
        if (dt == null) {
            time = 0;
            return;
        }
        if (us < 0) {
            us = 0;
        } else if (us >= 1000) {
            us = 999;
        }
        java.util.Calendar calendar = new java.util.GregorianCalendar();
        calendar.setTime(dt);
        time = calendar.get(java.util.Calendar.YEAR);
        time <<= 46; //18 bits for years
        long mid = calendar.get(java.util.Calendar.MONTH); //4 bits for month
        mid <<= 42;
        time += mid;
        mid = calendar.get(java.util.Calendar.DAY_OF_MONTH); //5 bits for day
        mid <<= 37;
        time += mid;
        mid = calendar.get(java.util.Calendar.HOUR_OF_DAY); //5 bits for hour
        mid <<= 32;
        time += mid;
        mid = calendar.get(java.util.Calendar.MINUTE); //6 bits for minute
        mid <<= 26;
        time += mid;
        mid = calendar.get(java.util.Calendar.SECOND); //6 bits for second
        mid <<= 20;
        time += mid;
        //20 bits for micro-seconds
        mid = calendar.get(java.util.Calendar.MILLISECOND);
        mid *= 1000;
        mid += us;
        time += mid;
    }
    private static final int MICRO_SECONDS = 0xfffff; //20 bits

    public java.util.Date get() {
        long dt = time;
        long us = (dt & MICRO_SECONDS);
        dt >>= 20;
        int sec = (int) (dt & 0x3f);
        dt >>= 6;
        int min = (int) (dt & 0x3f);
        dt >>= 6;
        int hour = (int) (dt & 0x1f);
        dt >>= 5;
        int day = (int) (dt & 0x1f);
        dt >>= 5;
        int mon = (int) (dt & 0xf);
        dt >>= 4;
        int year = (int) dt + 1900;
        java.util.GregorianCalendar gc = new java.util.GregorianCalendar(year, mon, day, hour, min, sec);
        dt = gc.getTime().getTime();
        int ms = (int) (us / 1000); //milli-seconds
        dt += ms;
        return new java.util.Date(dt);
    }
}
