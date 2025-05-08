package gtu.cse.cse396.sdbelt.scan.domain.model;

import java.time.LocalDateTime;
import java.util.UUID;

public record ProductStatistics(
        LocalDateTime startTime,
        LocalDateTime endTime,
        UUID productId,
        Long totalScanned,
        Long totalSuccess,
        Long totalFailed,
        double successRate,
        double failureRate
) {}