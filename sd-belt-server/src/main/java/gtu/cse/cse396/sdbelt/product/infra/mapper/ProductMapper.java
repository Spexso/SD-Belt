package gtu.cse.cse396.sdbelt.product.infra.mapper;

import org.springframework.stereotype.Component;
import gtu.cse.cse396.sdbelt.product.domain.model.Product;
import gtu.cse.cse396.sdbelt.product.infra.model.ProductEntity;

@Component
public class ProductMapper {

    private ProductMapper() {
        // Private constructor to prevent instantiation
    }
    
    public static Product toDomain(ProductEntity entity) {
        return new Product(
                entity.getId(),
                entity.getName(),
                entity.getDescription(),
                entity.getCreatedAt(),
                entity.getUpdatedAt()
        );
    }

    public static ProductEntity toEntity(Product product) {
        return new ProductEntity(
                product.id(),
                product.name(),
                product.description(),
                product.createdAt(),
                product.updatedAt()
        );
    }
}
