package gtu.cse.cse396.sdbelt.system.domain.model;

public record SystemLatestInfo(String timestamp, String level, String message) {

    @Override
    public String toString() {
        return "SystemLatestInfo{" +
                "timestamp='" + timestamp + '\'' +
                ", level='" + level + '\'' +
                ", message='" + message + '\'' +
                '}';
    }
}