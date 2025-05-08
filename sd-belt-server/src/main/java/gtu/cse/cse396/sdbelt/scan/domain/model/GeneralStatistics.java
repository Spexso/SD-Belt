package gtu.cse.cse396.sdbelt.scan.domain.model;

import java.time.LocalDateTime;
import java.util.List;

public record GeneralStatistics(
        LocalDateTime startTime,
        LocalDateTime endTime,
        Long totalScanned,
        Long totalSuccess,
        Long totalFailed,
        double successRate,
        double failureRate,
        List<ProductStatistics> productStatistics) {
}
