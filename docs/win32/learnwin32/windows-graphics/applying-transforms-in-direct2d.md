<!-- https://docs.microsoft.com/en-us/windows/win32/learnwin32/applying-transforms-in-direct2d -->

# Applying Transforms in Direct2D

In [Drawing with Direct2D](./drawing-with-direct2d.md), we saw that the `ID2D1RenderTarget::FillEllipse` method draws an ellipse that is aligned to the x- and y- axes. But suppose that you want to draw an ellipse tilted at an angle?

![](https://docs.microsoft.com/en-us/windows/win32/learnwin32/images/graphics16.png)

By using transforms, you can alter a shape in the following ways.

- Rotation around a point.
- Scaling.
- Translation (displacement in the X or Y direction).
- Skew (also known as _shear_).

A transform is a mathematical operation that maps a set of points to a new set of points. For example, the following diagram shows a triangle rotated around the point P3. After the rotation is applied, the point P1 is mapped to P1', the point P2 is mapped to P2', and the point P3 maps to itself.

![](https://docs.microsoft.com/en-us/windows/win32/learnwin32/images/graphics17.png)

Transforms are implemented by using matrices. However, you do not have to understand to mathematics of matrices in order to use them. If you want to learn more about the math, see [Appendix: Matrix Transforms](./appendix-matrix-transforms.md).

To apply a transform in Direct2D, call the `ID2D1RenderTarget::SetTransform` method. This method takes a `D2D1_MATRIX_3X2_F` structure that defines the transformation. You can initialize this structure by calling methods on the `D2D1::Matrix3x2F` class. This class contains static methods that return a matrix for each kind of transform:

- `Matrix3x2F::Rotation`
- `Matrix3x2F::Scale`
- `Matrix3x2F::Translation`
- `Matrix3x2F::Skew`

For example, the following code applies a 20-degree rotation around the point (100, 100).

```c++
pRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation(20, D2D1::Point2F(100, 100)));
```

The transform is applied to all later drawing operations until you call `SetTransform` again. To remove the current transform, call `SetTransform` with the identity matrix. To create the identity matrix, call the `Matrix3x2F::Identity` function.

```c++
pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
```

## Drawing Clock Hands

Let's put transforms to use by converting our Circle program into an analog clock. We can do this by adding lines for the hands.

![](https://docs.microsoft.com/en-us/windows/win32/learnwin32/images/graphics18.png)

Instead of calculating the coordinates for the lines, we can calculate the angle and then apply a rotation transform. The following code shows a function that draws one clock hand. The `fAngle` parameter gives the angle of the hand, in degrees.

```c++
void Scene::DrawClockHand(float fHandLength, float fAngle, float fStrokeWidth)
{
    m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation(fAngle, m_ellipse.point));

    // endPoint defines one end of the hand.
    D2D_POINT_2F endPoint = D2D1::Point2F(m_ellipse.point.x, m_ellipse.point.y - (m_ellipse.radiusY * fHandLength));

    // Draw a line from the center of the ellipse to endPoint.
    m_pRenderTarget->DrawLine(m_ellipse.point, endPoint, m_pStroke, fStrokeWidth);
}
```
