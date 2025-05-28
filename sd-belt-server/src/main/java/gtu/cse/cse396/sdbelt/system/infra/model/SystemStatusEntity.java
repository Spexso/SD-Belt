package gtu.cse.cse396.sdbelt.system.infra.model;

import jakarta.persistence.*;
import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

/**
 * JPA entity representing the system record in the conveyor belt systemning
 * system.
 * <p>
 * Each instance corresponds to a row in the {@code systems} table and captures
 * the
 * result of an individual system operation for a product.
 */
@Entity
@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
public class SystemStatusEntity {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    private String timestamp;

    private String level;

    private String message;
}
