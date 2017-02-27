
package SPA.ServerSide;
import java.lang.annotation.ElementType;
import java.lang.annotation.Target;

@Target({ElementType.FIELD})
@java.lang.annotation.Retention(java.lang.annotation.RetentionPolicy.RUNTIME)
public @interface ServiceAttr {
    public int ServiceID();
    public SPA.tagThreadApartment ThreadApartment() default SPA.tagThreadApartment.taNone;
}
