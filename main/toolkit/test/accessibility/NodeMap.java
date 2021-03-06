/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/

import com.sun.star.accessibility.XAccessibleContext;

import java.util.HashMap;

abstract class NodeMapCallback
{
    public abstract void Apply (AccTreeNode aNode);
}

/** This map translates from XAccessible objects to our internal
    representations.
*/
class NodeMap
{
    public NodeMap ()
    {
        maXAccessibleToNode = new HashMap ();
    }

    /** Clear the whole map.
    */
    public void Clear ()
    {
        maXAccessibleToNode.clear();
    }

    /** @return
            whether the new node was different from a previous one
            repspectively was the first one set.
    */
    public boolean InsertNode (XAccessibleContext xContext, AccessibleTreeNode aNode)
    {
        AccessibleTreeNode aPreviousNode = (AccessibleTreeNode)maXAccessibleToNode.put (
            xContext, 
            aNode);
        return aPreviousNode != aNode;
    }

    protected void RemoveNode (AccessibleTreeNode aNode)
    {
        try
        {
            if ((aNode != null) && (aNode instanceof AccTreeNode))
            {
                maXAccessibleToNode.remove (((AccTreeNode)aNode).getContext());
            }
        }
        catch (Exception e)
        {
            System.out.println ("caught exception while removing node " 
                + aNode + " : " + e);
            e.printStackTrace();
        }
    }


    public void ForEach (NodeMapCallback aFunctor)
    {
        Object[] aNodes = maXAccessibleToNode.values().toArray();
        for (int i=0; i<aNodes.length; i++)
        {
            if (aNodes[i] != null && (aNodes[i] instanceof AccTreeNode))
            {
                try
                {
                    aFunctor.Apply ((AccTreeNode)aNodes[i]);
                }
                catch (Exception e)
                {
                    System.out.println ("caught exception applying functor to " 
                        + i + "th node " + aNodes[i] + " : " + e);
                    e.printStackTrace();
                }
            }
        }
    }

    AccessibleTreeNode GetNode (XAccessibleContext xContext)
    {
        return (AccessibleTreeNode)maXAccessibleToNode.get (xContext);
    }

    AccessibleTreeNode GetNode (Object aObject)
    {
        if (aObject instanceof XAccessibleContext)
            return GetNode ((XAccessibleContext)aObject);
        else 
            return null;
    }

    XAccessibleContext GetAccessible (AccessibleTreeNode aNode)
    {
        if ((aNode != null) && (aNode instanceof AccTreeNode))
            return ((AccTreeNode)aNode).getContext();
        else 
            return null;
    }

    boolean IsMember (XAccessibleContext xContext)
    {
        return maXAccessibleToNode.containsKey(xContext);
    }

    boolean ValueIsMember (AccessibleTreeNode aNode)
    {
        return maXAccessibleToNode.containsValue(aNode);
    }



    private HashMap maXAccessibleToNode;
}
