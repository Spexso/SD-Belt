package gtu.cse.cse396.sdbelt.scan.domain.model;

import java.time.Instant;
import java.util.UUID;

import lombok.Builder;

/**
 * Represents the result of a product scan performed during the conveyor belt process.
 * <p>
 * Each scan captures the time of scanning, whether the operation was successful,
 * and an optional error message if the scan failed.
 */
@Builder
public record Scan(

    /**
     * The unique identifier of the product that was scanned.
     * <p>
     * This field links the scan result back to the product it belongs to.
     */
    UUID productId,

    /**
     * The timestamp when the scan was performed.
     * <p>
     * Indicates the exact date and time at which the scanning process took place.
     */
    Long timestamp,

    /**
     * Indicates whether the scan was successful.
     * <p>
     * If {@code true}, the scan succeeded and the product can be further processed.
     * If {@code false}, an error occurred and should be reviewed using {@link #errorMessage}.
     */
    Boolean isSuccess,

    /**
     * A human-readable message describing the scan error, if any.
     * <p>
     * This field is typically populated only if {@link #isSuccess} is {@code false}.
     */
    String errorMessage
) {}
