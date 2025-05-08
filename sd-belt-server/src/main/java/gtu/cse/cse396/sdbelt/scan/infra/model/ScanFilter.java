package gtu.cse.cse396.sdbelt.scan.infra.model;

import java.util.List;
import java.util.UUID;

import gtu.cse.cse396.sdbelt.shared.model.FilterTime;

/**
 * Encapsulates filtering and pagination criteria for retrieving scan records.
 * <p>
 * This filter is used by the {@link ScanService} to query scan data based on parameters
 * such as product ID, success status, sorting, and time ranges.
 *
 * @param size       the maximum number of scan records to return per page (for pagination); may be null for default
 * @param page       the zero-based index of the page to return; may be null for default
 * @param sort       a list of sorting criteria in the format {@code field,direction} (e.g. {@code timestamp,desc}); may be null or empty
 * @param productId  optional filter to restrict scans to a specific product ID
 * @param isSuccess  optional flag to filter by scan success status: {@code true}, {@code false}, or {@code null} for all
 * @param time       optional {@link FilterTime} object specifying a time range for filtering scans
 */
public record ScanFilter(
    Integer size,
    Integer page,
    List<String> sort,
    UUID productId,
    Boolean isSuccess,
    FilterTime time
) {}
