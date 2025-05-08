package gtu.cse.cse396.sdbelt.shared.model;

import java.util.List;

import org.springframework.data.domain.PageRequest;
import org.springframework.data.domain.Pageable;
import org.springframework.data.domain.Sort;


/**
 * Represents a generic filtering interface that provides pagination and sorting functionality.
 * This interface can be implemented by classes that need to support filtering, sorting, and pagination
 * when fetching data.
 */
public interface Filter {

    /**
     * Retrieves the size of the page.
     *
     * @return the number of records per page, or {@code null} if not specified.
     */
    Integer size();

    /**
     * Retrieves the page number.
     *
     * @return the current page number, or {@code null} if not specified.
     */
    Integer page();

    /**
     * Retrieves the sorting criteria.
     * The sorting criteria should be represented as a list of strings, where each entry follows the format:
     * {@code "fieldName,direction"} (e.g., "name,ASC" or "dateCreated,DESC").
     *
     * @return a list of sorting fields and directions, or {@code null} if sorting is not specified.
     */
    List<String> sort();

    /**
     * Constructs a {@link Pageable} object based on the provided pagination and sorting criteria.
     * If both {@code page()} and {@code size()} are provided, a {@link PageRequest} is created.
     * Otherwise, an unpaged {@link Pageable} instance is returned with sorting applied if specified.
     *
     * @return a {@link Pageable} object with pagination and sorting settings.
     */
    default Pageable pageable() {
        if (page() != null && size() != null) {
            return PageRequest.of(page(), size(), sortBy());
        }
        return PageRequest.of(0, 20, sortBy());
    }

    /**
     * Constructs a {@link Sort} object based on the provided sorting criteria.
     * If no sorting criteria are specified, {@link Sort#unsorted()} is returned.
     * Otherwise, the first entry in the sort list is used to determine the field and direction.
     * Sorting format: {@code "fieldName,direction"} where direction is either "ASC" or "DESC".
     *
     * @return a {@link Sort} object representing the sorting order.
     */
    default Sort sortBy() {
        if (sort() == null || sort().isEmpty()) {
            return Sort.unsorted();
        }
        String field = sort().get(0);
        String direction = sort().get(1).toUpperCase();
        return Sort.by(direction.equals("ASC") ? Sort.Order.asc(field) : Sort.Order.desc(field));
    }
}
