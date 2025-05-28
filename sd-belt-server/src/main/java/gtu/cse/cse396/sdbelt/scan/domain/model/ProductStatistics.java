package gtu.cse.cse396.sdbelt.scan.domain.model;

import java.time.LocalDateTime;

public record ProductStatistics(
        LocalDateTime startTime,
        LocalDateTime endTime,
        String productId,
        Long totalScanned,
        Long totalSuccess,
        Long totalFailed,
        double successRate,
        double failureRate) {
}