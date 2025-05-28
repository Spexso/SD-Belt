package gtu.cse.cse396.sdbelt.scan.domain.model;

import lombok.Builder;
import lombok.Getter;
import lombok.ToString;

/**
 * Represents the result of a product scan performed during the conveyor belt
 * process.
 * <p>
 * Each scan captures the time of scanning, whether the operation was
 * successful,
 * and an optional error message if the scan failed.
 */
@Builder
public record ScanRequestDTO(

        /**
         * The unique identifier of the product that was scanned.
         * <p>
         * This field links the scan result back to the product it belongs to.
         */
        String productResult,

        String confidence,

        String x,
        String y) {

    @Override
    public String toString() {
        return "ScanRequestDTO{" +
                "productResult='" + productResult + '\'' +
                ", confidence=" + confidence +
                ", x=" + x +
                ", y=" + y +
                '}';
    }
}
