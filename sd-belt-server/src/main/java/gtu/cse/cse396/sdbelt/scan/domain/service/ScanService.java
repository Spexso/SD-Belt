package gtu.cse.cse396.sdbelt.scan.domain.service;

import java.time.LocalDateTime;
import java.util.List;
import java.util.UUID;

import gtu.cse.cse396.sdbelt.scan.domain.model.GeneralStatistics;
import gtu.cse.cse396.sdbelt.scan.domain.model.ProductStatistics;
import gtu.cse.cse396.sdbelt.scan.domain.model.Scan;
import gtu.cse.cse396.sdbelt.scan.infra.model.ScanFilter;

/**
 * Service interface for managing scan records of products in the conveyor belt scanning system.
 * <p>
 * Provides methods for creating scan entries and retrieving scan histories,
 * supporting both full listing and filtered queries based on scan parameters.
 */
public interface ScanService {

    /**
     * Creates a new scan record for a product.
     *
     * @param productId    the UUID of the product that was scanned; must not be null
     * @param isSuccess    flag indicating whether the scan was successful; must not be null
     * @param errorMessage an optional error message; should be null or empty if {@code isSuccess} is true
     * @throws IllegalArgumentException if the product ID is invalid or missing required data
     */
    void create(UUID productId, Boolean isSuccess, String errorMessage);

    /**
     * Retrieves a list of all scan records in the system.
     *
     * @return a list of all {@link Scan} entries; never null, may be empty
     */
    List<Scan> list();

    /**
     * Finds scan records matching the provided filter criteria.
     *
     * @param filter a {@link ScanFilter} object containing search conditions such as date ranges,
     *               product IDs, or success status; must not be null
     * @return a list of scan records that match the filter; never null, may be empty
     */
    List<Scan> find(ScanFilter filter);


    /**
    * Generates general statistics for all scan records within the specified time range.
    *
    * @param startDate the beginning of the time range (inclusive); must not be null
    * @param endDate   the end of the time range (inclusive); must not be null and not before {@code startDate}
    * @return a {@link GeneralStatistics} object containing aggregated scan data (e.g., total scans, success/failure rate)
    * @throws IllegalArgumentException if the date range is invalid or any parameter is null
    */
    GeneralStatistics generateStatistics(LocalDateTime startDate, LocalDateTime endDate);

    /**
     * Generates statistics for a specific product within the specified time range.
     *
     * @param productId  the UUID of the product to generate statistics for; must not be null
     * @param startDate  the beginning of the time range (inclusive); must not be null
     * @param endDate    the end of the time range (inclusive); must not be null and not before {@code startDate}
     * @return a {@link ProductStatistics} object with metrics like scan count, error rate, and last scan timestamp
     * @throws IllegalArgumentException if any input is null or if the date range is invalid
     */
    ProductStatistics generateStatistics(UUID productId, LocalDateTime startDate, LocalDateTime endDate);
}
