package gtu.cse.cse396.sdbelt.scan.domain.model;

import java.time.LocalDateTime;

import lombok.Builder;
import lombok.Getter;

/**
 * Represents the result of a product scan performed during the conveyor belt
 * process.
 * <p>
 * Each scan captures the time of scanning, whether the operation was
 * successful,
 * and an optional error message if the scan failed.
 */
@Builder
public record Scan(

        String productId,

        Double healthRatio,

        Boolean isSuccess,

        String errorMessage,

        LocalDateTime timestamp) {
}
