package gtu.cse.cse396.sdbelt.shared.model;

public record FilterTime (
        String startTime,
        String endTime
) {
    public static FilterTime of(String startTime, String endTime) {
        return new FilterTime(startTime, endTime);
    }

    public static FilterTime empty() {
        return new FilterTime("", "");
    }
}